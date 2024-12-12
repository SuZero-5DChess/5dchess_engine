#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "multiverse.h"

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
    py::class_<board>(m, "board")
        .def(py::init())
        .def(py::init<std::string, int, int>(), 
             py::arg("fen"), 
             py::arg("x_size") = BOARD_LENGTH, 
             py::arg("y_size") = BOARD_LENGTH) // Constructor with parameters
        .def("get_piece", &board::get_piece)
        .def("set_piece", &board::set_piece)
        .def("__str__", &board::to_string);
    py::class_<multiverse>(m, "multiverse")
        .def(py::init<const std::string&>(), py::arg("input")) // Constructor
        .def("get_board", &multiverse::get_board, py::arg("t"), py::arg("l"), py::arg("c"),
             py::return_value_policy::reference) // Return shared_ptr to board
        .def("__str__", &multiverse::to_string) // String representation of the board
        .def("get_boards", &multiverse::get_boards)
        .def_readwrite("metadata", &multiverse::metadata); // Expose `metadata` map directly
}
