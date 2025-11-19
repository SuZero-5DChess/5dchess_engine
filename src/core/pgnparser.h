//
//  pgnparser.h
//  parser
//
//  Created by ftxi on 2025/10/26.
//

#ifndef pgnparser_h
#define pgnparser_h

#include <iostream>
#include <variant>
#include <optional>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

/*
 <move> ::= <physical-move> | <superphysical-move>
 <physical-move> ::= [<absolute-board>] ([<piece-name>] [<file>] [<rank>] ['x'] <file> <rank> ['=' <promote-to>] | 'O-O' | 'O-O-O')
 <superphysical-move> ::= [<absolute-board>] [<piece-name>] [<file>] [<rank>] (<jump-indicator> ['x'] | [<jump-indicator>] ['x'] <to-board>) <file> <rank> ['=' <promote-to>]
 <to-board> ::= <absote-board> | <relative-board>
 <absolute-board> ::= '(' (['L'] <line> | [['L'] <line>] 'T' <time>) ')'
 <relative-board> ::= '$(' (['L'] <difference> | [['L'] <difference>] 'T' <difference>) ')'
 <piece-name> ::= 'P' | 'W' | 'K' | 'C' | 'Q' | 'Y' | 'S' | 'N' | 'R' | 'B' | 'U' | 'D'
 <promote-to> ::= 'Q' ;change this if promotion to other pieces is allowed
 <jump-indicator> ::= '>' | '>>'
 <line> ::= ['+' | '-'] <natural-number>
 <time> ::= <natural-number>
 <difference> ::= '=' | '+' <positive-number> | '-' <positive-number>
 <file> ::= 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h'
 <rank> ::= '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8'
 <natural-number> ::= '0' | <positive-number>
 <positive-number> ::= '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | ...
*/

namespace pgnparser_defs {
typedef enum {
    NIL,
    WHITE_SPACE, COMMENT,
    TURN,
    LINE, TIME, RELATIVE_SYM, CAPTURE,
    PIECE, CASTLE_KINGSIDE, CASTLE_QUEENSIDE,
    FILE_CHAR,
    EQUAL, ZERO, POSITIVE_NUMBER,
    POSITIVE, NEGATIVE,
    NON_BRANCH_JUMP, BRANCHING_JUMP,
    LEFT_PAREN, RIGHT_PAREN,
    END
} token_t;

using turn_t = std::pair<int,bool>;

struct relative_board {
   std::optional<int> line_difference;
   std::optional<int> time_difference;
   friend std::ostream& operator<<(std::ostream& os, const relative_board& rb);
};
struct absolute_board {
   token_t sign;
   std::optional<int> line;
   std::optional<int> time;
   friend std::ostream& operator<<(std::ostream& os, const absolute_board& ab);
};
struct physical_move {
   std::optional<absolute_board> board;
   token_t castle;
   std::optional<char> piece_name;
   std::optional<char> from_file;
   std::optional<char> from_rank;
   bool capture;
   char to_file;
   char to_rank;
   std::optional<char> promote_to;
   friend std::ostream& operator<<(std::ostream& os, const physical_move& pm);
};
struct superphysical_move {
   std::optional<absolute_board> from_board;
   std::optional<char> piece_name;
   std::optional<char> from_file;
   std::optional<char> from_rank;
   token_t jump_indicater;
   bool capture;
   std::variant<std::monostate,absolute_board,relative_board> to_board;
   char to_file;
   char to_rank;
   std::optional<char> promote_to;
   friend std::ostream& operator<<(std::ostream& os, const superphysical_move& sm);
};
struct move {
   std::variant<physical_move, superphysical_move> data;
   friend std::ostream& operator<<(std::ostream& os, const move& mv);
};
struct actions {
    std::vector<move> moves;
    std::vector<std::string> comments;
    friend std::ostream& operator<<(std::ostream& os, const actions& ac);
};
struct gametree {
    actions act;
    std::vector<gametree> variations; // variations for next action
    friend std::ostream& operator<<(std::ostream& os, const gametree& gt);
};

} //namespace

class pgnparser
{
private:
    const bool check_turn_number;
    std::string input;
    struct {
        std::string::iterator current;
        pgnparser_defs::token_t token;
        char piece;
        char file;
        unsigned number;
        pgnparser_defs::turn_t turn;
        std::string_view comment;
    } buffer;
public:
    class parse_error : public std::runtime_error {
    public:
        explicit parse_error(const std::string& message)
            : std::runtime_error(message) {}
    };
    
    pgnparser(std::string msg, bool check_turn_number=true, pgnparser_defs::turn_t start_turn=std::make_pair(1,false));
    void next_token();
    void test_lexer();
     
    using relative_board = pgnparser_defs::relative_board;
    using absolute_board = pgnparser_defs::absolute_board;
    using physical_move = pgnparser_defs::physical_move;
    using superphysical_move = pgnparser_defs::superphysical_move;
    using move= pgnparser_defs::move;
    using actions = pgnparser_defs::actions;
    using gametree = pgnparser_defs::gametree;
    
    std::optional<relative_board> parse_relative_board();
    std::optional<absolute_board> parse_absolute_board();
    std::optional<physical_move> parse_physical_move();
    std::optional<superphysical_move> parse_superphysical_move();
    std::optional<move> parse_move();
private:
    // made private because the return value relies on the lifetime of this->input
    std::vector<std::string_view> parse_comments();
public:
    std::optional<actions> parse_actions();
    std::optional<gametree> parse_gametree();
    
    static bool match_absolute_board(absolute_board simple, absolute_board full);
    static bool match_relative_board(relative_board simple, relative_board full);
    static bool match_physical_move(physical_move a, physical_move b);
    static bool match_superphysical_move(superphysical_move a, superphysical_move b);
    static bool match_move(move a, move b);
};

#endif /* pgnparser_h */
