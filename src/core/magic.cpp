#include <array>
#include "magic.h"
#include "utils.h"

constexpr std::array<bitboard_t, BOARD_SIZE> rook_mask = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](int xy){
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1; nx<BOARD_LENGTH-1; nx++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, y);
        result |= pmask;
    }
    for(int nx = x-1; nx>=1; nx--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, y);
        result |= pmask;
    }
    for(int ny = y+1; ny<BOARD_LENGTH-1; ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(x, ny);
        result |= pmask;
    }
    for(int ny = y-1; ny>=1; ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(x, ny);
        result |= pmask;
    }
    return result;
});

constexpr std::array<bitboard_t, BOARD_SIZE> bishop_mask = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](int xy){
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1, ny=y+1; nx<BOARD_LENGTH-1 && ny<BOARD_LENGTH-1; nx++, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
    }
    for(int nx = x+1, ny=y-1; nx<BOARD_LENGTH-1 && ny>=1; nx++, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
    }
    for(int nx = x-1, ny=y+1; nx>=1 && ny<BOARD_LENGTH-1; nx--, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
    }
    for(int nx = x-1, ny=y-1; nx>=1 && ny>=1; nx--, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
    }
    return result;
});

consteval bitboard_t rook_attack_prototype(int xy, bitboard_t blocker)
{
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1; nx<BOARD_LENGTH; nx++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, y);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1; nx>=0; nx--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, y);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int ny = y+1; ny<BOARD_LENGTH; ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(x, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int ny = y-1; ny>=0; ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(x, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    return result;
}

consteval bitboard_t bishop_attack_prototype(int xy, bitboard_t blocker)
{
    bitboard_t result = 0;
    int x = xy%BOARD_LENGTH, y = xy/BOARD_LENGTH;
    for(int nx = x+1, ny=y+1; nx<BOARD_LENGTH && ny<BOARD_LENGTH; nx++, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x+1, ny=y-1; nx<BOARD_LENGTH && ny>=0; nx++, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1, ny=y+1; nx>=0 && ny<BOARD_LENGTH; nx--, ny++)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    for(int nx = x-1, ny=y-1; nx>=0 && ny>=0; nx--, ny--)
    {
        bitboard_t pmask = bitboard_t(1) << ppos(nx, ny);
        result |= pmask;
        if(pmask & blocker)
            break;
    }
    return result;
}

// same feature with std::popcount
constexpr int popcount(bitboard_t b)
{
    int r;
    for(r = 0; b; b &= b - 1)
        r++;
    return r;
}


constexpr std::array<int, BOARD_SIZE> rook_shift = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](int pos){
    return popcount(rook_mask[pos]);
});

constexpr std::array<int, BOARD_SIZE> bishop_shift = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](int pos){
    return popcount(bishop_mask[pos]);
});

constexpr int maximal_rook_shift = *std::max_element(rook_shift.begin(), rook_shift.end());
constexpr int maximal_bishop_shift = *std::max_element(bishop_shift.begin(), bishop_shift.end());

constexpr std::array<bitboard_t,64> rook_magic = {
    0x0a8002c000108020ull,
    0x06c00049b0002001ull,
    0x0100200010090040ull,
    0x2480041000800801ull,
    0x0280028004000800ull,
    0x0900410008040022ull,
    0x0280020001001080ull,
    0x2880002041000080ull,
    0xa000800080400034ull,
    0x0004808020004000ull,
    0x2290802004801000ull,
    0x0411000d00100020ull,
    0x0402800800040080ull,
    0x000b000401004208ull,
    0x2409000100040200ull,
    0x0001002100004082ull,
    0x0022878001e24000ull,
    0x1090810021004010ull,
    0x0801030040200012ull,
    0x0500808008001000ull,
    0x0a08018014000880ull,
    0x8000808004000200ull,
    0x0201008080010200ull,
    0x0801020000441091ull,
    0x0000800080204005ull,
    0x1040200040100048ull,
    0x0000120200402082ull,
    0x0d14880480100080ull,
    0x0012040280080080ull,
    0x0100040080020080ull,
    0x9020010080800200ull,
    0x0813241200148449ull,
    0x0491604001800080ull,
    0x0100401000402001ull,
    0x4820010021001040ull,
    0x0400402202000812ull,
    0x0209009005000802ull,
    0x0810800601800400ull,
    0x4301083214000150ull,
    0x204026458e001401ull,
    0x0040204000808000ull,
    0x8001008040010020ull,
    0x8410820820420010ull,
    0x1003001000090020ull,
    0x0804040008008080ull,
    0x0012000810020004ull,
    0x1000100200040208ull,
    0x430000a044020001ull,
    0x0280009023410300ull,
    0x00e0100040002240ull,
    0x0000200100401700ull,
    0x2244100408008080ull,
    0x0008000400801980ull,
    0x0002000810040200ull,
    0x8010100228810400ull,
    0x2000009044210200ull,
    0x4080008040102101ull,
    0x0040002080411d01ull,
    0x2005524060000901ull,
    0x0502001008400422ull,
    0x489a000810200402ull,
    0x0001004400080a13ull,
    0x4000011008020084ull,
    0x0026002114058042ull,
};

constexpr std::array<bitboard_t,64> bishop_magic = {
    0x89a1121896040240ull,
    0x2004844802002010ull,
    0x2068080051921000ull,
    0x62880a0220200808ull,
    0x0004042004000000ull,
    0x0100822020200011ull,
    0xc00444222012000aull,
    0x0028808801216001ull,
    0x0400492088408100ull,
    0x0201c401040c0084ull,
    0x00840800910a0010ull,
    0x0000082080240060ull,
    0x2000840504006000ull,
    0x30010c4108405004ull,
    0x1008005410080802ull,
    0x8144042209100900ull,
    0x0208081020014400ull,
    0x004800201208ca00ull,
    0x0f18140408012008ull,
    0x1004002802102001ull,
    0x0841000820080811ull,
    0x0040200200a42008ull,
    0x0000800054042000ull,
    0x88010400410c9000ull,
    0x0520040470104290ull,
    0x1004040051500081ull,
    0x2002081833080021ull,
    0x000400c00c010142ull,
    0x941408200c002000ull,
    0x0658810000806011ull,
    0x0188071040440a00ull,
    0x4800404002011c00ull,
    0x0104442040404200ull,
    0x0511080202091021ull,
    0x0004022401120400ull,
    0x80c0040400080120ull,
    0x8040010040820802ull,
    0x0480810700020090ull,
    0x0102008e00040242ull,
    0x0809005202050100ull,
    0x8002024220104080ull,
    0x0431008804142000ull,
    0x0019001802081400ull,
    0x0200014208040080ull,
    0x3308082008200100ull,
    0x041010500040c020ull,
    0x4012020c04210308ull,
    0x208220a202004080ull,
    0x0111040120082000ull,
    0x6803040141280a00ull,
    0x2101004202410000ull,
    0x8200000041108022ull,
    0x0000021082088000ull,
    0x0002410204010040ull,
    0x0040100400809000ull,
    0x0822088220820214ull,
    0x0040808090012004ull,
    0x00910224040218c9ull,
    0x0402814422015008ull,
    0x0090014004842410ull,
    0x0001000042304105ull,
    0x0010008830412a00ull,
    0x2520081090008908ull,
    0x40102000a0a60140ull,
};

constexpr size_t rook_data_size = size_t(1) << (maximal_rook_shift + BOARD_BITS*2);

consteval std::array<bitboard_t, rook_data_size> gen_rook_data()
{
    std::array<bitboard_t, rook_data_size> data{};
    for(int pos = 0; pos < BOARD_SIZE; pos++)
    {
        const bitboard_t mask = rook_mask[pos];
        // this is the layman's way to iterate all submasks...
        int positions[maximal_rook_shift], n = 0;
        for(int i = 0; i < 64; ++i)
        {
            if (mask & (bitboard_t(1) << i))
            {
                positions[n++] = i;
            }
        }
        
        bitboard_t total_subsets = bitboard_t(1) << n;
        for(bitboard_t subset = 0; subset < total_subsets; ++subset)
        {
            bitboard_t submask = 0;
            for(int j = 0; j < n; ++j)
            {
                if(subset & (bitboard_t(1) << j))
                {
                    submask |= (bitboard_t(1) << positions[j]);
                }
            }
            // do stuff with submask
            size_t index = ((submask & mask) * rook_magic[pos]) >> (64 - rook_shift[pos]);
            data[index<<(BOARD_BITS*2) | pos] = rook_attack_prototype(pos, submask);
        }
    }
    return data;
}
constexpr auto rook_data = gen_rook_data();

bitboard_t rook_attack(int pos, bitboard_t blocker)
{
    size_t index = ((blocker & rook_mask[pos]) * rook_magic[pos]) >> (64 - rook_shift[pos]);
    return rook_data[index<<(BOARD_BITS*2) | pos];
}

constexpr size_t bishop_data_size = size_t(1) << (maximal_bishop_shift + BOARD_BITS*2);

consteval std::array<bitboard_t, bishop_data_size> gen_bishop_data()
{
    std::array<bitboard_t, bishop_data_size> data{};
    for(int pos = 0; pos < BOARD_SIZE; pos++)
    {
        const bitboard_t mask = bishop_mask[pos];
        // this is the layman's way to iterate all submasks...
        int positions[maximal_bishop_shift], n = 0;
        for(int i = 0; i < 64; ++i)
        {
            if (mask & (bitboard_t(1) << i))
            {
                positions[n++] = i;
            }
        }
        
        bitboard_t total_subsets = bitboard_t(1) << n;
        for(bitboard_t subset = 0; subset < total_subsets; ++subset)
        {
            bitboard_t submask = 0;
            for(int j = 0; j < n; ++j)
            {
                if(subset & (bitboard_t(1) << j))
                {
                    submask |= (bitboard_t(1) << positions[j]);
                }
            }
            // do stuff with submask
            size_t index = ((submask & mask) * bishop_magic[pos]) >> (64 - bishop_shift[pos]);
            data[index<<(BOARD_BITS*2) | pos] = bishop_attack_prototype(pos, submask);
        }
    }
    return data;
}
constexpr auto bishop_data = gen_bishop_data();

bitboard_t bishop_attack(int pos, bitboard_t blocker)
{
    size_t index = ((blocker & bishop_mask[pos]) * bishop_magic[pos]) >> (64 - bishop_shift[pos]);
    return bishop_data[index<<(BOARD_BITS*2) | pos];
}


bitboard_t queen_attack(int pos, bitboard_t blocker)
{
    return rook_attack(pos, blocker) | bishop_attack(pos, blocker);
}
