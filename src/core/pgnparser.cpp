#include "pgnparser.h"


//#define DEBUGMSG

template <typename T>
void debug_print_impl(T t)
{
    std::cerr << t << "\n";
}

template<typename T, typename ...Args>
void debug_print_impl(T t, Args... args)
{
    std::cerr << t << " ";
    debug_print_impl(args...);
}

template<typename ...Args>
void dprint(Args... args)
{
#ifdef DEBUGMSG
    std::cerr << "[DEBUG] ";
    debug_print_impl(args...);
#endif
}

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

} // namespace

using namespace pgnparser_defs;

pgnparser::pgnparser(std::string msg) : input(msg)
{
    buffer.current = input.begin();
    next_token();
}

void pgnparser::next_token()
{
    while(buffer.current != input.end() && isspace(*buffer.current))
        buffer.current++;
    if(buffer.current == input.end())
    {
        buffer.token = END;
        dprint("buffer.token:END");
        return;
    }
    switch(*buffer.current)
    {
        case 'L':
            buffer.token = LINE; dprint("buffer.token:LINE"); buffer.current++; break;
        case 'T':
            buffer.token = TIME; dprint("buffer.token:TIME"); buffer.current++; break;
        case '$':
            buffer.token = RELATIVE_SYM; dprint("buffer.token:RELATIVE_SYM"); buffer.current++; break;
        case 'x':
            buffer.token = CAPTURE; dprint("buffer.token:CAPTURE"); buffer.current++; break;
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
            dprint("buffer.token:PIECE", buffer.piece);
            buffer.current++;
            break;
        case 'O':
            if(input.end()-buffer.current>=2 && *++buffer.current == '-' && *++buffer.current == 'O')
            {
                if(input.end()-buffer.current>=2 && *++buffer.current == '-' && *++buffer.current == 'O')
                {
                    buffer.token = CASTLE_QUEENSIDE;
                    dprint("buffer.token:CASTLE_QUEENSIDE");
                    buffer.current++;
                    break;
                }
                else
                {
                    buffer.token = CASTLE_KINGSIDE;
                    dprint("buffer.token:CASTLE_KINGSIDE");
                    buffer.current++;
                    break;
                }
            }
            dprint("error: expected castling after O, got", std::string(input.begin(),buffer.current), "{<<-this}", std::string(buffer.current,input.end()));
            throw parse_error("unknown buffer.token");
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
            dprint("buffer.token:FILE_CHAR", buffer.file);
            buffer.current++;
            break;
        case '=':
            buffer.token = EQUAL; dprint("buffer.token:EQUAL"); buffer.current++; break;
        case '0':
            buffer.token = ZERO; dprint("buffer.token:ZERO"); buffer.current++; break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            buffer.token = POSITIVE_NUMBER;
            buffer.number = 0;
            while(buffer.current != input.end() && '0' <= *buffer.current && *buffer.current <= '9')
            {
                buffer.number = buffer.number*10 + (*buffer.current-'0');
                buffer.current++;
            }
            dprint("buffer.token:POSITIVE_NUMBER", buffer.number);
            break;
        case '+':
            buffer.token = POSITIVE; dprint("buffer.token:POSITIVE"); buffer.current++; break;
        case '-':
            buffer.token = NEGATIVE; dprint("buffer.token:NEGATIVE"); buffer.current++; break;
        case '>':
            buffer.current++;
            if(*buffer.current == '>')
            {
                buffer.token = BRANCHING_JUMP;
                buffer.current++;
                dprint("buffer.token:BRANCHING_JUMP");
            }
            else
            {
                buffer.token = NON_BRANCH_JUMP;
                dprint("buffer.token:NON_BRANCH_JUMP");
            }
            break;
        case '(':
            buffer.token = LEFT_PAREN; dprint("buffer.token:LEFT_PAREN"); buffer.current++; break;
        case ')':
            buffer.token = RIGHT_PAREN; dprint("buffer.token:RIGHT_PAREN"); buffer.current++; break;
        default:
            dprint("error: ", *buffer.current);
            throw parse_error("unknown buffer.token");
    }
}


#define PARSE_START auto fallback = buffer;
#define PARSE_FAIL { \
    buffer = fallback;\
    dprint("fallback to", std::string(input.begin(),fallback.current), "{<<-here}", std::string(fallback.current, input.end())); \
    return std::nullopt; \
}
#define PARSED_MSG std::string(fallback.current, buffer.current)

// <relative-to-board> ::= '$(' (['L'] <difference> 'T' <difference> | 'L' <difference> | 'T' <difference>) ')'
// <difference> ::= '=' | '+' <positive-buffer.number> | '-' <positive-buffer.number>
std::optional<relative_board> pgnparser::parse_relative_board()
{
    PARSE_START;  // cache old buffer.current pointer in case it fails
    dprint("parse relative board");
    std::optional<int> ldiff, tdiff;
    bool has_line = true;
    if(buffer.token != RELATIVE_SYM) PARSE_FAIL;
    next_token();
    if(buffer.token != LEFT_PAREN) PARSE_FAIL;
    next_token();
    if(buffer.token == LINE) next_token(); // skip ['L']
    int sign = 0;
    switch (buffer.token) {
        case POSITIVE: sign = 1; next_token(); break;
        case NEGATIVE: sign = -1; next_token(); break;
        case EQUAL: ldiff = 0; next_token(); break;
        default: has_line = false;
    }
    if(has_line && buffer.token == POSITIVE_NUMBER)
    {
        ldiff = sign*buffer.number;
        next_token();
    }
    if(buffer.token == TIME)
    {
        next_token();
        switch (buffer.token) {
            case POSITIVE: sign = 1; break;
            case NEGATIVE: sign = -1; break;
            case EQUAL: tdiff = 0; break;
            default: PARSE_FAIL;
        }
        next_token();
        if(buffer.token == POSITIVE_NUMBER)
        {
            tdiff = sign*buffer.number;
            next_token();
        }
    }
    else if(!has_line) PARSE_FAIL;
    if(buffer.token != RIGHT_PAREN) PARSE_FAIL;
    next_token();
        
    if(!(ldiff.has_value() || tdiff.has_value()))
        throw parse_error("Relative board without any LT information: " + PARSED_MSG);
    return relative_board{ldiff, tdiff};
}

// <absolute-to-board> ::= '(' ['L'] <line> 'T' <time> ')' | '(L' <line> ')' | '(T' <time> ')'
// <line> ::= ['+' | '-'] <natural-buffer.number>
std::optional<absolute_board> pgnparser::parse_absolute_board()
{
    PARSE_START;  // cache old buffer.current pointer in case it fails
    dprint("parse absolute board");
    token_t sign = NIL;
    std::optional<int> l, t;
    bool has_line = true;
    if(buffer.token != LEFT_PAREN) PARSE_FAIL;
    next_token();
    if(buffer.token == LINE) next_token(); // skip ['L']
    switch (buffer.token) {
        case POSITIVE:
        case NEGATIVE:
            sign = buffer.token; next_token(); break;
        case POSITIVE_NUMBER: break;
        case ZERO: break;
        default: has_line = false;
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
        else PARSE_FAIL;
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
        else PARSE_FAIL;
    }
    else if(!has_line) PARSE_FAIL;
    if(buffer.token != RIGHT_PAREN) PARSE_FAIL;
    next_token();
        
    if(!(l.has_value() || t.has_value()))
        throw parse_error("Absolute board without any LT information: " + PARSED_MSG);
    return absolute_board{sign, l, t};
}

// <physical-move> ::= [<absolute-board>] ([<buffer.piece-name>] [<file>] [<rank>] ['x'] <file> <rank> ['=' <promote-to>] | 'O-O' | 'O-O-O')
// <to-board> ::= <absote-board> | <relative-board>
std::optional<physical_move> pgnparser::parse_physical_move()
{
    PARSE_START;
    dprint("parse physical move");
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
    if(from_file.has_value() && from_rank.has_value()
        && (buffer.token == EQUAL || is_move_separator(buffer.token)))
    {
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
        if(buffer.token != PIECE) PARSE_FAIL;
        promote_to = buffer.piece;
        next_token();
    }
    return physical_move{board, NIL, piece_name, from_file, from_rank, capture, to_file, to_rank, promote_to};
}

// <superphysical-move> ::= [<absolute-board>] [<buffer.piece-name>] [<file>] [<rank>] (<jump-indicator> ['x'] | [<jump-indicator>] ['x'] <to-board>) <file> <rank>
// <to-board> ::= <absote-board> | <relative-board>
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
    switch(buffer.token)
    {
        case LEFT_PAREN:
            if(auto tmp = parse_absolute_board())
            {
                to_board = *tmp;
            }
            else PARSE_FAIL;
            break;
        case RELATIVE_SYM:
            if(auto tmp = parse_relative_board())
            {
                to_board = *tmp;
            }
            else PARSE_FAIL;
            break;
        case FILE_CHAR:
            to_board = std::monostate();
            break;
        default:
            PARSE_FAIL;
    }
    if(std::holds_alternative<std::monostate>(to_board) && jump_indicater==NIL)
        PARSE_FAIL;
    if(buffer.token != FILE_CHAR) PARSE_FAIL;
    to_file = buffer.file;
    next_token();
    if(buffer.token != POSITIVE_NUMBER) PARSE_FAIL;
    to_rank = buffer.number;
    next_token();
    if(buffer.token == PIECE)
    {
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
