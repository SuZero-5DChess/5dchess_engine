#include <iostream>
#include "game.h"
#include "utils.h"
#include <vector>
#include <variant>
#include "bitboard.h"
#include "magic.h"

#include <random>
#include <cassert>

std::string standard_fen = ""
"r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*";

 using namespace std;

using uint64 = unsigned long long;

bitboard_t ratt(int xy, bitboard_t blocker)
{
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1; nx<BOARD_LENGTH; nx++)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, y);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1; nx>=0; nx--)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, y);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int ny = y+1; ny<BOARD_LENGTH; ny++)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(x, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int ny = y-1; ny>=0; ny--)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(x, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    return result;
}

bitboard_t batt(int xy, bitboard_t blocker)
{
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1, ny=y+1; nx<BOARD_LENGTH && ny<BOARD_LENGTH; nx++, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x+1, ny=y-1; nx<BOARD_LENGTH && ny>=0; nx++, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1, ny=y+1; nx>=0 && ny<BOARD_LENGTH; nx--, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1, ny=y-1; nx>=0 && ny>=0; nx--, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << board::ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    return result;
}

void ASSERT_EQ(auto lhs, auto rhs)
{
    if(lhs != rhs)
    {
        std::cout << "assertion failed:\n";
        std::cout << "lhs = " << lhs << std::endl;
        std::cout << "rhs = " << rhs << std::endl;
    }
    assert(lhs == rhs);
}

void test_attacks()
{
    std::random_device rd;  // Seed source
    std::mt19937_64 gen(rd()); // 64-bit Mersenne Twister PRNG
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX); // Uniform distribution
    
    for(int i = 0; i < 100000; i++)
    {
        uint64_t random_blocker = dist(gen); // Generate 64-bit random number
        int random_entry = dist(gen) % 64;
        ASSERT_EQ(rook_attack(random_entry, random_blocker), ratt(random_entry, random_blocker));
        //ASSERT_EQ(bishop_attack_prototype(random_entry, random_blocker), batt(random_entry, random_blocker));
    }
    cerr << "test passed" << endl;
}

 int main()
 {
     test_attacks();
//     board b(standard_fen);
//     bb_full_collection bb(b);
//     print_range("", bishop_shift);
//     cout << endl;
//     uint64_t x = uint64_t(1) << 63;
//     std::cout << sizeof(rook_data) << endl;
//     std::cout << bitboard_to_string(rook_data[919]) << endl;
     std::cout << "shutting down" << std::endl;
     return 0;
 }
