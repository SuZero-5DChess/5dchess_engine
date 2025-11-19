#include "pgnparser.h"
#include "utils.h"
#include <sstream>

//#define DEBUGMSG
#include "debug.h"

template <typename T>
bool match_opt(const std::optional<T> &simple, const std::optional<T> &full)
{
    if(simple)
        return simple == full;
    else
        return true;
}

namespace pgnparser_defs {

std::ostream &operator<<(std::ostream &os, const relative_board &rb)
{
    return os << "rb{ld:" << (rb.line_difference ? std::to_string(*rb.line_difference) : "?")
                << ",td:" << (rb.time_difference ? std::to_string(*rb.time_difference) : "?") << "}";
}

std::ostream &operator<<(std::ostream &os, const absolute_board &ab)
{
    return os << "ab{s:" << (ab.sign == POSITIVE ? "+" : ab.sign == NEGATIVE ? "-" : std::to_string(ab.sign))
                << ",l:" << (ab.line ? std::to_string(*ab.line) : "?")
                << ",t:" << (ab.time ? std::to_string(*ab.time) : "?") << "}";
}

std::ostream &operator<<(std::ostream &os, const physical_move &pm)
{
    os << "pm{b:";
    if(pm.board) {os << *pm.board;} else {os << "?";}
    if(pm.castle==CASTLE_KINGSIDE) return os << ",castle_kingside}";
    if(pm.castle==CASTLE_QUEENSIDE) return os << ",castle_queenside}";
    os << ",pn:" << (pm.piece_name ? *pm.piece_name : '?');
    os << ",ff:" << (pm.from_file ? *pm.from_file : '?');
    os << ",fr:" << (pm.from_rank ? std::to_string(*pm.from_rank) : "?");
    os << ",cp:" << (pm.capture ? "yes" : "no");
    os << ",tf:" << pm.to_file << ",tr:" << static_cast<int>(pm.to_rank);
    os << ",pt:" << (pm.promote_to ? *pm.promote_to : '?')  << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const superphysical_move &sm)
{
    os << "sm{fb:";
    if(sm.from_board) {os << *sm.from_board;} else {os << "?";}
    os << ",pn:" << (sm.from_file ? *sm.from_file : '?');
    os << ",ff:" << (sm.from_file ? *sm.from_file : '?');
    os << ",fr:" << (sm.from_rank ? std::to_string(*sm.from_rank) : "?");
    os << ",ji:" << (sm.jump_indicater == NON_BRANCH_JUMP ? ">" :
                        sm.jump_indicater == BRANCHING_JUMP ? ">>" : "?");
    os << ",cp:" << (sm.capture ? "yes" : "no");
    os << ",tb:";
    if(std::holds_alternative<absolute_board>(sm.to_board))
        os << std::get<absolute_board>(sm.to_board);
    else if(std::holds_alternative<relative_board>(sm.to_board))
        os << std::get<relative_board>(sm.to_board);
    else
        os << "?";
    os << ",tf:" << sm.to_file << ",tr:" << static_cast<int>(sm.to_rank);
    os << ",pt:" << (sm.promote_to ? *sm.promote_to : '?')  << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const move &mv)
{
    os << "mv:";
    if(std::holds_alternative<physical_move>(mv.data))
        os << std::get<physical_move>(mv.data);
    else
        os << std::get<superphysical_move>(mv.data);
    return os;
}

std::ostream &operator<<(std::ostream &os, const actions &ac)
{
    os << "action{moves:";
    os << range_to_string(ac.moves, "[", "]");
    os << ", comments:";
    os << range_to_string(ac.comments, "[", "]");
    os << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const gametree &gt)
{
    os << "gametree{act:" << gt.act;
    os << ", var:";
    os << range_to_string(gt.variations, "[", "]");
    os << "}";
    return os;
}

} // namespace

using namespace pgnparser_defs;

inline turn_t next_turn(turn_t t)
{
    auto [n,c] = t;
    int v = (n<<1 | c) + 1;
    return std::make_pair(v>>1, v&1);
}

inline turn_t previous_turn(turn_t t)
{
    auto [n,c] = t;
    int v = (n<<1 | c) - 1;
    return std::make_pair(v>>1, v&1);
}

std::vector<std::string> views_to_strings(const std::vector<std::string_view>& views)
{
    std::vector<std::string> result(views.size());
    std::transform(views.begin(), views.end(), result.begin(),
                   [](std::string_view v) {
                       return std::string(v);
                   });
    return result;
}

pgnparser::pgnparser(std::string msg, bool ck, turn_t start_turn) : check_turn_number(ck), input(msg)
{
    buffer.current = input.begin();
    buffer.turn = previous_turn(start_turn);
    next_token();
}

void pgnparser::next_token()
{
    
    if(buffer.current == input.end())
    {
        buffer.token = END;
        dprint("token:END");
        return;
    }
    switch(*buffer.current)
    {
        case 'L':
            buffer.token = LINE; dprint("token:LINE"); buffer.current++; break;
        case 'T':
            buffer.token = TIME; dprint("token:TIME"); buffer.current++; break;
        case '$':
            buffer.token = RELATIVE_SYM; dprint("token:RELATIVE_SYM"); buffer.current++; break;
        case 'x':
            buffer.token = CAPTURE; dprint("token:CAPTURE"); buffer.current++; break;
        case 'P':
        case 'W':
        case 'K':
        case 'C':
        case 'Q':
        case 'Y':
        case 'S':
        case 'N':
        case 'R':
        case 'B':
        case 'U':
        case 'D':
            buffer.token = PIECE;
            buffer.piece = *buffer.current;
            dprint("token:PIECE", buffer.piece);
            buffer.current++;
            break;
        case 'O':
            if(input.end()-buffer.current>=2 && *++buffer.current == '-' && *++buffer.current == 'O')
            {
                if(input.end()-buffer.current>=2 && *++buffer.current == '-' && *++buffer.current == 'O')
                {
                    buffer.token = CASTLE_QUEENSIDE;
                    dprint("token:CASTLE_QUEENSIDE");
                    buffer.current++;
                    break;
                }
                else
                {
                    buffer.token = CASTLE_KINGSIDE;
                    dprint("token:CASTLE_KINGSIDE");
                    buffer.current++;
                    break;
                }
            }
            else
            {
                std::ostringstream oss;
                oss << "next_token(): expected castling after O, got ";
                oss << std::string(input.begin(),buffer.current) << "{<<-this}";
                oss << std::string(buffer.current,input.end());
                throw parse_error(oss.str());
            }
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
            buffer.token = FILE_CHAR;
            buffer.file = *buffer.current;
            dprint("token:FILE_CHAR", buffer.file);
            buffer.current++;
            break;
        case '=':
            buffer.token = EQUAL; dprint("token:EQUAL"); buffer.current++; break;
        case '0':
            buffer.token = ZERO; dprint("token:ZERO"); buffer.current++; break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            buffer.number = 0;
            while(buffer.current != input.end() && '0' <= *buffer.current && *buffer.current <= '9')
            {
                buffer.number = buffer.number*10 + (*buffer.current-'0');
                buffer.current++;
            }
            if(buffer.current!=input.end()
               && (*buffer.current == 'w' || *buffer.current == 'b')
               && (buffer.current+1 != input.end())
               && (*(buffer.current+1) == '.'))
            {
                bool c = *buffer.current == 'b';
                buffer.current++;
                auto newturn = std::make_pair(buffer.number, c);
                if(check_turn_number && newturn != next_turn(buffer.turn))
                {
                    std::ostringstream oss;
                    oss << "next_token(): non-consecutive turn number: ";
                    auto [oldn, oldc] = buffer.turn;
                    oss << oldn << (oldc ? "b" : "w") << " followed by ";
                    oss << buffer.number << (c ? "b" : "w") << "\n";
                    throw parse_error(oss.str());
                }
                dprint("token:TURN", buffer.number, c ? "b" : "w");
                buffer.token = TURN;
                buffer.turn = newturn;
                buffer.current++;
            }
            else if(buffer.current!=input.end() && *buffer.current == '.')
            {
                 const bool c = false;
                 auto newturn = std::make_pair(buffer.number, c);
                 if(check_turn_number && newturn != next_turn(buffer.turn))
                 {
                     std::ostringstream oss;
                     oss << "next_token(): non-consecutive turn number: ";
                     auto [oldn, oldc] = buffer.turn;
                     oss << oldn << (oldc ? "b" : "w") << " followed by ";
                     oss << buffer.number << (c ? "b" : "w") << "\n";
                     throw parse_error(oss.str());
                 }
                 dprint("token:TURN", buffer.number, c ? "b" : "w");
                 buffer.token = TURN;
                 buffer.turn = newturn;
                 buffer.current++;
            }
            else
            {
                dprint("token:POSITIVE_NUMBER", buffer.number);
                buffer.token = POSITIVE_NUMBER;
            }
            break;
        case '/':
            buffer.turn = next_turn(buffer.turn);
            dprint("token:TURN /", buffer.turn.first, buffer.turn.second?"b":"w");
            buffer.token = TURN;
            buffer.current++;
            break;
        case '+':
            buffer.token = POSITIVE; dprint("token:POSITIVE"); buffer.current++; break;
        case '-':
            buffer.token = NEGATIVE; dprint("token:NEGATIVE"); buffer.current++; break;
        case '>':
            buffer.current++;
            if(*buffer.current == '>')
            {
                buffer.token = BRANCHING_JUMP;
                buffer.current++;
                dprint("token:BRANCHING_JUMP");
            }
            else
            {
                buffer.token = NON_BRANCH_JUMP;
                dprint("token:NON_BRANCH_JUMP");
            }
            break;
        case '(':
            buffer.token = LEFT_PAREN; dprint("token:LEFT_PAREN"); buffer.current++; break;
        case ')':
            buffer.token = RIGHT_PAREN; dprint("token:RIGHT_PAREN"); buffer.current++; break;
        case '{':
        {
            std::string::iterator start = buffer.current;
            int nest_level = 1;
            while(nest_level > 0)
            {
                buffer.current++;
                if(buffer.current==input.end())
                    throw parse_error("next_token(): comment not closed: "+ std::string(start, buffer.current));
                else if(*buffer.current == '{')
                    nest_level++;
                else if(*buffer.current == '}')
                    nest_level--;
            }
            buffer.comment = std::string_view(start+1, buffer.current);
            buffer.token = COMMENT;
            dprint("token:COMMENT", buffer.comment);
            buffer.current++;
            break;
        }
        default:
            if(isspace(*buffer.current))
            {
                buffer.token = WHITE_SPACE;
                dprint("token:WHITE_SPACE");
                while(buffer.current != input.end() && isspace(*buffer.current))
                    buffer.current++;
            }
            else
            {
                std::ostringstream oss;
                oss << "next_token(): unknown buffer.token: " << *buffer.current;
                throw parse_error(oss.str());
            }
    }
}


#define PARSE_START auto fallback = buffer;
#define PARSE_FAIL { \
    buffer = fallback;\
    dprint("fallback to", std::string(input.begin(),fallback.current), "{<<-here}", std::string(fallback.current, input.end()), "\n[  ...]", __func__, "failed"); \
    return std::nullopt; \
}
#define PARSED_MSG std::string(fallback.current, buffer.current)

void pgnparser::test_lexer()
{
    while(buffer.token != END)
        next_token();
}

/*
 <relative-to-board> ::= '$(' (['L'] <difference> 'T' <difference> | 'L' <difference> | 'T' <difference>) ')'
 <difference> ::= '=' | '+' <positive-buffer.number> | '-' <positive-buffer.number>
 throws error if detected '$(' but the following is not <relative-to-board> syntax
 */
std::optional<relative_board> pgnparser::parse_relative_board()
{
    PARSE_START;  // cache old buffer.current pointer in case it fails
    dprint("parse_relative_board()");
    std::optional<int> ldiff, tdiff;
    bool has_line = true, expect_line = false;
    if(buffer.token != RELATIVE_SYM) PARSE_FAIL;
    next_token();
    if(buffer.token != LEFT_PAREN) PARSE_FAIL;
    next_token();
    if(buffer.token == LINE)
    {
        expect_line = true;
        next_token(); // skip ['L']
    }
    int sign = 0;
    switch (buffer.token) {
        case POSITIVE: sign = 1; next_token(); break;
        case NEGATIVE: sign = -1; next_token(); break;
        case EQUAL: ldiff = 0; next_token(); break;
        default:
            if(expect_line)
                throw parse_error("parse_relative_board(): Expect line difference after 'L': " + PARSED_MSG);
            else
                has_line = false;
    }
    if(has_line)
    {
        if(sign != 0 && buffer.token == POSITIVE_NUMBER)
        {
            ldiff = sign*buffer.number;
            next_token();
        }
        else if(sign == 0 && buffer.token == POSITIVE_NUMBER)
            throw parse_error("parse_relative_board(): No number should appear after '=': " + PARSED_MSG);
        else if(sign != 0 && buffer.token != POSITIVE_NUMBER)
            throw parse_error("parse_relative_board(): Expect number after '+/-': " + PARSED_MSG);
    }
    if(buffer.token == TIME)
    {
        next_token();
        
        switch (buffer.token) {
            case POSITIVE: sign = 1; break;
            case NEGATIVE: sign = -1; break;
            case EQUAL: tdiff = 0; break;
            default:
                throw parse_error("parse_relative_board(): Expect turn difference after 'L': " + PARSED_MSG);
        }
        next_token();
        if(buffer.token == POSITIVE_NUMBER)
        {
            if(sign != 0 && buffer.token == POSITIVE_NUMBER)
            {
                tdiff = sign*buffer.number;
                next_token();
            }
            else if(sign == 0 && buffer.token == POSITIVE_NUMBER)
                throw parse_error("parse_relative_board(): No number should appear after '=': " + PARSED_MSG);
            else if(sign != 0 && buffer.token != POSITIVE_NUMBER)
                throw parse_error("parse_relative_board(): Expect number after '+'/'-': " + PARSED_MSG);
        }
    }
    else if(!has_line)
        throw parse_error("parse_relative_board(): Relative board without any LT information: " + PARSED_MSG);
    if(buffer.token != RIGHT_PAREN)
        throw parse_error("parse_relative_board(): Expect ')': " + PARSED_MSG);
    next_token();
    
    return relative_board{ldiff, tdiff};
}

/*
 <absolute-to-board> ::= '(' ['L'] <line> 'T' <time> ')' | '(L' <line> ')' | '(T' <time> ')'
 <line> ::= ['+' | '-'] <natural-number>
 throws error if:
 - parsed '(L' but the next is not a <line>
 - parsed '(L+' or '(L-' but the next is not a number
 - parsed '(' <line> 'T' or '(L' <line> 'T' or '(T' but next is not a positive number
 - parsed everything except for ')', which is missing
 fail instead of throw an error in examples such as '(2. Kf7)' or '(16b.Qxg5 17.'
*/
std::optional<absolute_board> pgnparser::parse_absolute_board()
{
    PARSE_START;  // cache old buffer.current pointer in case it fails
    dprint("parse_absolute_board()");
    token_t sign = NIL;
    std::optional<int> l, t;
    bool has_line = true, expect_line = false;
    if(buffer.token != LEFT_PAREN) PARSE_FAIL;
    next_token();
    if(buffer.token == LINE)
    {
        expect_line = true;
        next_token(); // skip ['L']
    }
    switch (buffer.token) {
        case POSITIVE:
        case NEGATIVE:
            sign = buffer.token; next_token(); break;
        case POSITIVE_NUMBER: break;
        case ZERO: break;
        default:
            if(expect_line)
                throw parse_error("parse_absolute_board(): Expect line after 'L': " + PARSED_MSG);
            else
                has_line = false;
    }
    if(has_line)
    {
        if(buffer.token == POSITIVE_NUMBER)
        {
            l = buffer.number;
            next_token();
        }
        else if (buffer.token == ZERO)
        {
            l = 0;
            next_token();
        }
        else if (sign == NIL)
        {
            PARSE_FAIL;
        }
        else
        {
            throw parse_error("parse_absolute_board(): Expect number after '+'/'-': " + PARSED_MSG);
        }
    }
    if(buffer.token == TIME)
    {
        next_token();
        if(buffer.token == POSITIVE_NUMBER)
        {
            t = buffer.number;
            next_token();
        }
        else if (buffer.token == ZERO)
        {
            t = 0;
            next_token();
        }
        else
            throw parse_error("parse_absolute_board(): Expect number after 'T': " + PARSED_MSG);;
    }
    else if(!has_line)
        PARSE_FAIL;
    if(buffer.token != RIGHT_PAREN) PARSE_FAIL;
    next_token();
    
    return absolute_board{sign, l, t};
}

/*
 <physical-move> ::= [<absolute-board>] ([<piece-name>] [<file>] [<rank>] ['x'] <file> <rank> ['=' <promote-to>] | 'O-O' | 'O-O-O')
 <to-board> ::= <absote-board> | <relative-board>
 throw an excption if '=' found in the end but no promotion piece given
*/
std::optional<physical_move> pgnparser::parse_physical_move()
{
    PARSE_START;
    dprint("parse_physical_move()");
    std::optional<absolute_board> board;
    std::optional<char> piece_name;
    std::optional<char> from_file;
    std::optional<char> from_rank;
    bool capture = false;
    char to_file;
    char to_rank;
    std::optional<char> promote_to;
    
    board = parse_absolute_board();
    if(buffer.token == CASTLE_KINGSIDE || buffer.token == CASTLE_QUEENSIDE)
    {
        token_t castle = buffer.token;
        next_token();
        return physical_move{board, castle, std::nullopt, std::nullopt, std::nullopt, false, 0, 0, std::nullopt};
    }
    if(buffer.token == PIECE)
    {
        piece_name = buffer.piece;
        next_token();
    }
    if(buffer.token == FILE_CHAR)
    {
        from_file = buffer.file;
        next_token();
    }
    if(buffer.token == POSITIVE_NUMBER)
    {
        from_rank = buffer.number;
        next_token();
    }
    if(from_file.has_value() && from_rank.has_value() && buffer.token != CAPTURE
       && buffer.token != FILE_CHAR)
    {
        if(buffer.token == NON_BRANCH_JUMP || buffer.token == BRANCHING_JUMP
           || parse_relative_board().has_value() || parse_absolute_board().has_value())
            PARSE_FAIL;
        dprint("short physical move");
        to_file = *from_file; from_file = std::nullopt;
        to_rank = *from_rank; from_rank = std::nullopt;
    }
    else
    {
        dprint("long physical move");
        if(buffer.token == CAPTURE)
        {
            capture = true;
            next_token();
        }
        if(buffer.token != FILE_CHAR) PARSE_FAIL;
        to_file = buffer.file;
        next_token();
        if(buffer.token != POSITIVE_NUMBER) PARSE_FAIL;
        to_rank = buffer.number;
        next_token();
    }
    if(buffer.token == EQUAL)
    {
        next_token();
        if(buffer.token != PIECE)
            throw parse_error("parse_physical_move(): expect promotion piece after '=': " + PARSED_MSG);
        promote_to = buffer.piece;
        next_token();
    }
    return physical_move{board, NIL, piece_name, from_file, from_rank, capture, to_file, to_rank, promote_to};
}

/*
 <superphysical-move> ::= [<absolute-board>] [<piece-name>] [<file>] [<rank>] (<jump-indicator> ['x'] | [<jump-indicator>] ['x'] <to-board>) <file> <rank>
 <to-board> ::= <absote-board> | <relative-board>
 throw an excption if there parsed through and found one of <jump-indicator> or <to-board>
 but the rest is not <superphysical-move> syntax
 */
std::optional<superphysical_move> pgnparser::parse_superphysical_move()
{
    PARSE_START;
    dprint("parse superphysical move");
    std::optional<absolute_board> from_board;
    std::optional<char> piece_name;
    std::optional<char> from_file;
    std::optional<char> from_rank;
    token_t jump_indicater;
    bool capture = false;
    std::variant<std::monostate,absolute_board,relative_board> to_board;
    char to_file;
    char to_rank;
    std::optional<char> promote_to;
    
    from_board = parse_absolute_board();
    if(buffer.token == PIECE)
    {
        piece_name = buffer.piece;
        next_token();
    }
    if(buffer.token == FILE_CHAR)
    {
        from_file = buffer.file;
        next_token();
    }
    if(buffer.token == POSITIVE_NUMBER)
    {
        from_rank = buffer.number;
        next_token();
    }
    if(buffer.token == NON_BRANCH_JUMP || buffer.token == BRANCHING_JUMP)
    {
        jump_indicater = buffer.token;
        next_token();
    }
    else
        jump_indicater = NIL;
    if(buffer.token == CAPTURE)
    {
        capture = true;
        next_token();
    }
    if(jump_indicater != NIL && buffer.token == FILE_CHAR)
    {
        to_board = std::monostate();
    }
    else if(auto tmp = parse_absolute_board())
    {
        to_board = *tmp;
    }
    else if(auto tmp = parse_relative_board())
    {
        to_board = *tmp;
    }
    else if(jump_indicater == NIL)
    {
        PARSE_FAIL;
    }
    else
    {
        throw parse_error("parse_superphysical_move(): expect destination board/squre after '>'/'>>': " + PARSED_MSG);
    }
    // if parsing does not fail before this position, we believe this shouldn't be something
    // other than a superphysical move
    if(buffer.token != FILE_CHAR)
        throw parse_error("parse_superphysical_move(): expect destination file: " + PARSED_MSG);
    to_file = buffer.file;
    next_token();
    if(buffer.token != POSITIVE_NUMBER)
        throw parse_error("parse_superphysical_move(): expect destination rank: " + PARSED_MSG);
    to_rank = buffer.number;
    next_token();
    if(buffer.token == EQUAL)
    {
        next_token();
        if(buffer.token != PIECE)
            throw parse_error("parse_superphysical_move(): expect promotion piece after '=': " + PARSED_MSG);
        promote_to = buffer.piece;
        next_token();
    }
    return superphysical_move{from_board, piece_name, from_file, from_rank, jump_indicater, capture, to_board, to_file, to_rank, promote_to};
}

std::optional<move> pgnparser::parse_move()
{
    auto pm = parse_physical_move();
    if(pm)
        return move{*pm};
    else
    {
        auto sm = parse_superphysical_move();
        if(sm)
            return move{*sm};
        else
            return std::nullopt;
    }
}

/*
 parses <space-comment>*
 <space-comment> ::= <whitespace> | <comment>
 iterate through all consecutive spaces/comments and return all comments found
*/
std::vector<std::string_view> pgnparser::parse_comments()
{
    dprint("parse_comments()");
    std::vector<std::string_view> comments;
    while(buffer.token == WHITE_SPACE || buffer.token == COMMENT)
    {
        if(buffer.token == COMMENT)
            comments.push_back(buffer.comment);
        next_token();
    }
    return comments;
}

//<actions> ::= <turn-serial> <space-comment>* <move> {<space-comment>+ <move>}* <space-comment>*
std::optional<actions> pgnparser::parse_actions()
{
    PARSE_START;
    if(buffer.token != TURN) PARSE_FAIL;
    turn_t turn = buffer.turn;
    next_token();
    while(buffer.token == WHITE_SPACE || buffer.token == COMMENT)
    {
        next_token();
    }
    std::vector<move> moves;
    std::vector<std::string_view> comments;
    comments.clear();
    std::optional<move> mv_buffer = parse_move();
    if(!mv_buffer.has_value()) PARSE_FAIL;
    moves.push_back(*mv_buffer);
    if(buffer.token != WHITE_SPACE && buffer.token != COMMENT)
        return actions{moves, views_to_strings(comments)};
    else
        comments = parse_comments();
    while((mv_buffer = parse_move()))
    {
        moves.push_back(*mv_buffer);
        comments.clear();
        if(buffer.token != WHITE_SPACE && buffer.token != COMMENT)
            break;
        else
            comments = parse_comments();
    }
    return actions{moves, views_to_strings(comments)};
}

/*
 <game-tree> ::= <actions> <space-comment>* {'(' <space-comment>* <game-tree>')' <space-comment>*}* [<game-tree> <space-comment>*]
 */
std::optional<gametree> pgnparser::parse_gametree()
{
    PARSE_START;
    std::optional<actions> act_opt = parse_actions();
    std::vector<gametree> variations;
    std::optional<gametree> var_buffer;
    if(!act_opt.has_value()) PARSE_FAIL;
    parse_comments();
    turn_t branch_start_turn = buffer.turn;
    while (buffer.token == LEFT_PAREN)
    {
        next_token();
        parse_comments();
        var_buffer = parse_gametree();
        if(!var_buffer.has_value())
            throw parse_error("parse_gametree(): invalid game tree branch: " + PARSED_MSG);
        else if(buffer.token != RIGHT_PAREN)
            throw parse_error("parse_gametree(): expect ')':" + PARSED_MSG);
        else
            variations.push_back(*var_buffer);
        buffer.turn = branch_start_turn; //last branch is over, reset turn number
        dprint("turn reset to:", buffer.turn.first, buffer.turn.second?"b":"w");
        next_token();
        parse_comments();
    }
    var_buffer = parse_gametree();
    if(var_buffer)
    {
        variations.push_back(*var_buffer);
    }
    return gametree{*act_opt, variations};
}

/* ***MATCHER*** */

bool pgnparser::match_absolute_board(absolute_board simple, absolute_board full)
{
    dprint("match_absolute_board");
    return (!simple.line || std::pair{simple.sign, simple.line} == std::pair{full.sign, full.line})
        && match_opt(simple.time, full.time);
}

bool pgnparser::match_relative_board(relative_board simple, relative_board full)
{
    dprint("match_relative_board");
    return match_opt(simple.line_difference, full.line_difference)
        && match_opt(simple.time_difference, full.time_difference);
}

bool pgnparser::match_physical_move(physical_move a, physical_move b)
{
    dprint("match_physical_move");
#ifdef DEBUGMSG
    std::cerr << "board=" << (a.board ? match_absolute_board(*a.board, *b.board) : true) << ' ';
#endif
    if(!(a.board ? match_absolute_board(*a.board, *b.board) : true))
        return false;
    if(a.castle==CASTLE_KINGSIDE)
        return b.castle == CASTLE_KINGSIDE
        || (b.piece_name == 'K' && b.from_file == 'e' && b.from_rank == 1 && b.to_file == 'g' && b.to_rank == 1)
        || (b.piece_name == 'K' && b.from_file == 'e' && b.from_rank == 8 && b.to_file == 'g' && b.to_rank == 8);
    else if(a.castle==CASTLE_QUEENSIDE)
        return b.castle == CASTLE_QUEENSIDE
        || (b.piece_name == 'K' && b.from_file == 'e' && b.from_rank == 1 && b.to_file == 'c' && b.to_rank == 1)
        || (b.piece_name == 'K' && b.from_file == 'e' && b.from_rank == 8 && b.to_file == 'c' && b.to_rank == 8);
#ifdef DEBUGMSG
    std::cerr
        << "board="        << (a.board ? match_absolute_board(*a.board, *b.board) : true) << ' '
        << "piece_name="   << match_opt(a.piece_name, b.piece_name) << ' '
        << "from_file="    << match_opt(a.from_file, b.from_file) << ' '
        << "from_rank="    << match_opt(a.from_rank, b.from_rank) << ' '
        << "capture="      << (!a.capture || b.capture) << ' '
        << "to_file="      << (a.to_file == b.to_file) << ' '
        << "to_rank="      << (a.to_rank == b.to_rank) << ' '
        << "promote_to="   << match_opt(a.promote_to, b.promote_to) << '\n';
#endif
    return match_opt(a.piece_name, b.piece_name)
        && match_opt(a.from_file, b.from_file)
        && match_opt(a.from_rank, b.from_rank)
        && (!a.capture || b.capture)
        && a.to_file == b.to_file
        && a.to_rank == b.to_rank
        && match_opt(a.promote_to, b.promote_to);
}

bool pgnparser::match_superphysical_move(superphysical_move a, superphysical_move b)
{
    dprint("match_superphysical_move");
    bool tb_match;
    if(std::holds_alternative<std::monostate>(a.to_board))
        tb_match = true;
    else if(a.to_board.index() != b.to_board.index())
        tb_match = false;
    else if(std::holds_alternative<absolute_board>(a.to_board))
        tb_match = match_absolute_board(std::get<absolute_board>(a.to_board), std::get<absolute_board>(b.to_board));
    else// if(std::holds_alternative<relative_board>(a.to_board))
        tb_match = match_relative_board(std::get<relative_board>(a.to_board), std::get<relative_board>(b.to_board));
#ifdef DEBUGMSG
    std::cerr
        << "from_board="     << (a.from_board ? match_absolute_board(*a.from_board, *b.from_board) : true) << ' '
        << "piece_name="     << match_opt(a.piece_name, b.piece_name) << ' '
        << "from_file="      << match_opt(a.from_file, b.from_file) << ' '
        << "from_rank="      << match_opt(a.from_rank, b.from_rank) << ' '
        << "jump_indicater=" << (a.jump_indicater == NIL || a.jump_indicater == b.jump_indicater) << ' '
        << "tb_match="       << tb_match << ' '
        << "capture="        << (!a.capture || b.capture) << ' '
        << "to_file="        << (a.to_file == b.to_file) << ' '
        << "to_rank="        << (a.to_rank == b.to_rank) << ' '
        << "promote_to="     << match_opt(a.promote_to, b.promote_to) << '\n';
#endif
    return (a.from_board ? match_absolute_board(*a.from_board, *b.from_board) : true)
        && match_opt(a.piece_name, b.piece_name)
        && match_opt(a.from_file, b.from_file)
        && match_opt(a.from_rank, b.from_rank)
        && (a.jump_indicater == NIL || a.jump_indicater == b.jump_indicater)
        && tb_match
        && (!a.capture || b.capture)
        && a.to_file == b.to_file
        && a.to_rank == b.to_rank
        && match_opt(a.promote_to, b.promote_to);
}

bool pgnparser::match_move(move a, move b)
{
    dprint("match_move");
    if(a.data.index() != b.data.index())
        return false;
    else if(std::holds_alternative<physical_move>(a.data))
        return pgnparser::match_physical_move(std::get<physical_move>(a.data), std::get<physical_move>(b.data));
    else
        return pgnparser::match_superphysical_move(std::get<superphysical_move>(a.data), std::get<superphysical_move>(b.data));
}
