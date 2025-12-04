#include "game.h"
#include <regex>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <cassert>
#include <variant>
#include "pgnparser.h"
#include "hypercuboid.h"

game::game(std::unique_ptr<gnode<comments_t>> gt)
: gametree{std::move(gt)}, current_node{gametree.get()}, cached_states{}, cached_moves{}
{
    cached_states.push_back(current_node->get_state());
    now = cached_states.begin();
    now_moves = cached_moves.begin();
}

game game::from_pgn(std::string input)
{
    auto ag = pgnparser(input).parse_game();
    if(!ag.has_value())
        throw std::runtime_error("Bad input, parse failed");
    pgnparser_ast::gametree gt_ast = std::move(ag->gt);
    ag->gt = {};
    game g(gnode<comments_t>::create_root(state(*ag), comments_t{}));
    g.metadata = ag->headers;
    gnode<comments_t> *cn = nullptr;
    // parse moves
    std::function<void(gnode<comments_t>*, const pgnparser_ast::gametree&)> dfs;
    dfs = [&dfs, &cn](gnode<comments_t>* node, const pgnparser_ast::gametree& gt_ast) -> void {
        for(const auto& [act_ast, child_gt] : gt_ast.variations)
        {
            state s = node->get_state();
            std::vector<ext_move> moves;
            for(const auto& mv_ast: act_ast.moves)
            {
                auto [fm_opt, pt_opt, candidates] = s.parse_move(mv_ast);
                if(!fm_opt.has_value())
                {
                    if(candidates.empty())
                    {
                        std::ostringstream oss;
                        oss << "state(): Invalid move: " << mv_ast;
                        
                        throw std::runtime_error(oss.str());
                    }
                    else
                    {
                        std::ostringstream oss;
                        oss << "state(): Ambiguous move: " << mv_ast << "; candidates: ";
                        oss << range_to_string(candidates, "", "");
                        throw std::runtime_error(oss.str());
                    }
                }
                else
                {
                    full_move fm = fm_opt.value();
                    bool flag;
                    if(pt_opt.has_value())
                    {
                        flag = s.apply_move<false>(fm, *pt_opt);
                    }
                    else
                    {
                        flag = s.apply_move<false>(fm);
                    }
                    if(!flag)
                    {
                        std::ostringstream oss;
                        oss << "state(): Illegal move: " << mv_ast << " (parsed as: " << fm << ")";
                        throw std::runtime_error(oss.str());
                    }
                    moves.push_back(ext_move(fm, pt_opt.has_value() ? *pt_opt : QUEEN_W));
                }
            }
            bool flag = s.submit();
            if(!flag)
            {
                std::ostringstream oss;
                oss << "state(): Cannot submit after parsing these moves: " << act_ast;
                throw std::runtime_error(oss.str());
            }
            action act = action::from_vector(moves, node->get_state());
            std::unique_ptr<gnode<comments_t>> child_node = gnode<comments_t>::create_child(node, s, act, act_ast.comments);
            gnode<comments_t>* child_node_ptr = node->add_child(std::move(child_node));
            cn = child_node_ptr;
            dfs(child_node_ptr, *child_gt);
        }
    };
    
    dfs(g.gametree.get(), gt_ast);
    if(cn)
        g.current_node = cn;
    g.fresh();
    return g;
}

void game::fresh()
{
    cached_states.clear();
    cached_moves.clear();
    cached_states.push_back(current_node->get_state());
    now = cached_states.begin();
    now_moves = cached_moves.begin();
}

std::tuple<int,int> game::get_current_present() const
{
    return get_current_state().get_present();
}

state game::get_current_state() const
{
    return *now;
}

std::vector<std::tuple<int, int, bool, std::string>> game::get_current_boards() const
{
    return get_current_state().get_boards();
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> game::get_current_timeline_status() const
{
    return get_current_state().get_timeline_status();
}

std::vector<vec4> game::gen_move_if_playable(vec4 p)
{
    if(is_playable(p))
    {
        const state& cs = get_current_state();
        std::vector<vec4> result;
        for(vec4 v : cs.gen_piece_move(p))
        {
            result.push_back(v);
        }
        return result;
    }
    else
    {
        return std::vector<vec4>();
    }
}

match_status_t game::get_match_status() const
{
    return match_status_t();
}

std::vector<vec4> game::get_movable_pieces() const
{
    state s = get_current_state();
    return s.gen_movable_pieces();
}

bool game::is_playable(vec4 p) const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_current_timeline_status();
    const state& cs = get_current_state();
    if (std::find(mandatory_timelines.begin(), mandatory_timelines.end(), p.l()) != mandatory_timelines.end() ||
        std::find(optional_timelines.begin(), optional_timelines.end(), p.l()) != optional_timelines.end()) 
    {
        auto [t, c] = cs.get_present();
        auto v1 = std::make_pair(p.t(), c);
        auto v2 = cs.get_timeline_end(p.l());
        if(v1 == v2)
        {
            piece_t p_piece = cs.get_piece(p, c);
            if(p_piece != NO_PIECE && p_piece != WALL_PIECE)
            {
                return c == static_cast<int>(piece_color(p_piece));
            }
        }
    }
    return false;
}

bool game::can_undo() const
{
    return now != cached_states.begin();
}

bool game::can_redo() const
{
    return now+1 != cached_states.end();
}

bool game::can_submit() const
{
    return get_current_state().can_submit().has_value();
}

bool game::undo()
{
    bool flag = can_undo();
    if(flag)
    {
        now--;
        now_moves--;
    }
    return flag;
}

bool game::redo()
{
    bool flag = can_undo();
    if(flag)
    {
        now++;
        now_moves++;
    }
    return flag;
}

bool game::apply_move(ext_move m)
{
    std::optional<state> ans = now->can_apply(m.fm, m.promote_to);
    if(ans)
    {
        state new_state = std::move(ans.value());
        cached_states.erase(now + 1, cached_states.end());
        cached_states.push_back(new_state);
        now = cached_states.end() - 1;
        if(now_moves < cached_moves.end())
            cached_moves.erase(now_moves + 1, cached_moves.end());
        cached_moves.push_back(m);
        now_moves = cached_moves.end() - 1;
        return true;
    }
    return false;
}

bool game::submit()
{
    std::optional<state> ans = now->can_submit();
    if(ans)
    {
        visit_child(action::from_vector(cached_moves, *now));
        return true;
    }
    return false;
}

bool game::currently_check() const
{
    auto [t, c] = get_current_state().get_present();
    return get_current_state().find_checks(!c).first().has_value();
}

std::vector<std::pair<vec4, vec4>> game::get_current_checks() const
{
    auto [t, c] = get_current_state().get_present();
    std::vector<std::pair<vec4, vec4>> result;
    for(full_move fm : get_current_state().find_checks(!c))
    {
        result.push_back(std::make_pair(fm.from, fm.to));
    }
    return result;
}

std::tuple<int, int> game::get_board_size() const
{
    const auto [size_x, size_y] = get_current_state().get_board_size();
    return std::make_tuple(size_x, size_y);
}

bool game::suggest_action()
{
    const state &s = current_node->get_state();
    auto [w, ss] = HC_info::build_HC(s);
    for(moveseq mvs : w.search(ss))
    {
        std::vector<ext_move> emvs;
        std::transform(mvs.begin(), mvs.end(), std::back_inserter(emvs), [](full_move m){
            return ext_move(m);
        });
        action act = action::from_vector(emvs, s);
        if(!current_node->find_child(act))
        {
            visit_child(act);
            visit_parent();
            return true;
        }
    }
    return false;
}

/////////////////////////////
// Comments and navigation //
/////////////////////////////

game::comments_t game::get_comments() const
{
    return current_node->get_info();
}

bool game::has_parent() const
{
    return current_node->get_parent() != nullptr;
}

void game::visit_parent()
{
    if(!has_parent())
        return;
    current_node = current_node->get_parent();
    cached_states.clear();
    cached_states.push_back(current_node->get_state());
    now = cached_states.begin();
}

std::vector<std::tuple<action, std::string>> game::get_child_moves() const
{
    std::vector<std::tuple<action, std::string>> result;
    auto &children = current_node->get_children();
    state s = current_node->get_state();
    for(const auto &child : children)
    {
        const action &act = child->get_action();
        std::string txt = s.pretty_action(act);
        result.push_back({act, txt});
    }
    return result;
}

bool game::visit_child(action act, comments_t comments, std::optional<state> newstate)
{
    // check if the child already exists
    auto &children = current_node->get_children();
    for(auto &child : children)
    {
        if(child->get_action().get_moves() == act.get_moves())
        {
            current_node = child.get();
            fresh();
            return true;
        }
    }
    // create a new child
    auto new_child = gnode<comments_t>::create_child(current_node, newstate, act, comments);
    current_node = current_node->add_child(std::move(new_child));
    fresh();
    return false;
}
