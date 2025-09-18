#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/operators.h>
#include <ostream>
#include "game.h"

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
    m.doc() = "5d chess engine"; // optional module docstring
    py::enum_<piece_t>(m, "Piece")
        .value("NO_PIECE", NO_PIECE)
        .value("WALL_PIECE", WALL_PIECE)
        .value("KING_UW", KING_UW)
        .value("ROOK_UW", ROOK_UW)
        .value("PAWN_UW", PAWN_UW)
        .value("KING_UB", KING_UB)
        .value("ROOK_UB", ROOK_UB)
        .value("PAWN_UB", PAWN_UB)
        .value("KING_W", KING_W)
        .value("QUEEN_W", QUEEN_W)
        .value("BISHOP_W", BISHOP_W)
        .value("KNIGHT_W", KNIGHT_W)
        .value("ROOK_W", ROOK_W)
        .value("PAWN_W", PAWN_W)
        .value("UNICORN_W", UNICORN_W)
        .value("DRAGON_W", DRAGON_W)
        .value("BRAWN_W", BRAWN_W)
        .value("PRINCESS_W", PRINCESS_W)
        .value("ROYAL_QUEEN_W", ROYAL_QUEEN_W)
        .value("COMMON_KING_W", COMMON_KING_W)
        .value("KING_B", KING_B)
        .value("QUEEN_B", QUEEN_B)
        .value("BISHOP_B", BISHOP_B)
        .value("KNIGHT_B", KNIGHT_B)
        .value("ROOK_B", ROOK_B)
        .value("PAWN_B", PAWN_B)
        .value("UNICORN_B", UNICORN_B)
        .value("DRAGON_B", DRAGON_B)
        .value("BRAWN_B", BRAWN_B)
        .value("PRINCESS_B", PRINCESS_B)
        .value("ROYAL_QUEEN_B", ROYAL_QUEEN_B)
        .value("COMMON_KING_B", COMMON_KING_B)
        .export_values();  // Exports the values for easy access
    py::enum_<match_status_t>(m, "match_status_t")
        .value("PLAYING", match_status_t::PLAYING)
        .value("WHITE_WINS", match_status_t::WHITE_WINS)
        .value("BLACK_WINS", match_status_t::BLACK_WINS)
        .value("STALEMATE", match_status_t::STALEMATE)
        .def("__str__", [](match_status_t status) {
            std::ostringstream oss;
            oss << status;
            return oss.str();
        });
    /*
    py::class_<board>(m, "board")
        .def(py::init<std::string, int, int>(), 
             py::arg("fen"), 
             py::arg("x_size") = BOARD_LENGTH, 
             py::arg("y_size") = BOARD_LENGTH) // Constructor with parameters
        .def("get_piece", &board::get_piece)
        .def("set_piece", &board::set_piece)
        .def("__str__", &board::to_string);
    */
   /*
    py::class_<multiverse>(m, "multiverse")
        .def(py::init<const std::string&>(), py::arg("input")) // Constructor
        .def("get_board", &multiverse::get_board, py::arg("l"), py::arg("t"), py::arg("c"),
             py::return_value_policy::reference) // Return shared_ptr to board
        .def("__str__", &multiverse::to_string) // String representation of the board
        .def("get_boards", &multiverse::get_boards)
        .def("get_piece", &multiverse::get_piece)
        .def("gen_piece_move", &multiverse::gen_piece_move)
        .def_readwrite("metadata", &multiverse::metadata); // Expose `metadata` map directly
    */
    py::class_<multiverse, std::shared_ptr<multiverse>>(m, "multiverse")
        // Bind the constructor
        .def(py::init<const std::string &>(), py::arg("input"))
        // Bind public methods
        .def("get_board", &multiverse::get_board, py::arg("l"), py::arg("t"), py::arg("c"))
        .def("get_boards", &multiverse::get_boards)
        .def("to_string", &multiverse::to_string)
        .def("inbound", &multiverse::inbound, py::arg("a"), py::arg("color"))
        .def("get_piece", &multiverse::get_piece, py::arg("a"), py::arg("color"))
        .def("gen_piece_move", &multiverse::gen_piece_move, py::arg("p"), py::arg("board_color"));
        // // Bind public member variables
        // .def_readwrite("metadata", &multiverse::metadata);
    /*
    py::class_<vec4>(m, "vec4")
        .def(py::init<int, int, int, int>(), py::arg("x"), py::arg("y"), py::arg("t"), py::arg("l"))
        .def("__repr__", &vec4::to_string);
        //.def("__add__", &vec4::operator+); 
    */
    py::class_<vec4>(m, "vec4")
        // Bind the constructor
        .def(py::init<int, int, int, int>(), py::arg("x"), py::arg("y"), py::arg("t"), py::arg("l"))
        // Bind member functions
        .def("l", &vec4::l)
        .def("t", &vec4::t)
        .def("y", &vec4::y)
        .def("x", &vec4::x)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self >= py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self < py::self)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self * int())
        //.def(int() * py::self) //this should be a bug
        .def("to_string", &vec4::to_string)
        // Bind the stream operator for string representation
        .def("__repr__", [](const vec4 &v) { return v.to_string(); });
    py::class_<full_move>(m, "full_move")
        .def(py::init<std::string>())
        .def_static("submit", &full_move::submit)
        .def_static("move", &full_move::move)
        .def("nonempty", &full_move::nonempty)
        .def("__lt__", &full_move::operator<)
        .def("__eq__", &full_move::operator==)
        .def("__repr__", [](const full_move& fm) {
            std::ostringstream os;
            os << fm;
            return os.str();
        });
    py::class_<state>(m, "state")
        .def_readwrite("m", &state::m)
        .def(py::init<multiverse>())
        .def("apply_move", &state::apply_move<false>);
    py::class_<game>(m, "game")
        .def(py::init<std::string>())
        .def("get_current_boards", &game::get_current_boards)
        .def("get_current_present", &game::get_current_present)
        .def("get_current_timeline_status", &game::get_current_timeline_status)
        .def("gen_move_if_playable", &game::gen_move_if_playable)
        .def("get_match_status", &game::get_match_status)
        .def("get_movable_pieces", &game::get_movable_pieces)
        .def("is_playable", &game::is_playable)
        .def("can_undo", &game::can_undo)
        .def("can_redo", &game::can_redo)
        .def("can_submit", &game::can_submit)
        .def("undo", &game::undo)
        .def("redo", &game::redo)
        .def("apply_move", &game::apply_move, py::return_value_policy::reference)
        .def("currently_check", &game::currently_check)
        .def("get_current_checks", &game::get_current_checks)
        .def("get_board_size", &game::get_board_size)
        .def_readwrite("metadata", &game::metadata);  // Expose metadata map
}
