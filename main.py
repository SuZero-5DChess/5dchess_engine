from build import engine
import host

fen = """
[Size "8x8"]
[Board  "custom"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:1:b]
[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:2:w]
[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/2B5/4P3/P*P*P*P*1P*P*P*/R*NBQK*1NR*:0:2:b]
[r*nbqk*b1r*/p*p*p*1p*p*p*p*/3p4/8/2B5/4P3/P*P*P*P*1P*P*P*/R*NBQK*1NR*:0:3:w]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:w]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/P7/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:b]
[r*nbqk*bnr*/p*1p*p*p*p*p*p*/6n1/1p6/P7/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:2:w]
[r*nbqk*bnr*/p*1p*p*p*p*p*p*/6n1/1P6/8/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:2:b]
[r*nbqk*bnr*/p*2p*p*p*p*p*/6n1/1Pp5/8/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:3:w]
"""

def convert_boards_data(boards):
    def convert_board(board):
        l, t, c, s = board
        return {"l":l, "t":t, "c":c, "fen":s}
    return list(map(convert_board, boards))

if __name__ == '__main__':
    m = engine.multiverse(fen)
    pos = engine.vec4(2, 3, 3, 0)
    c = 0
    moves = [{'x':p.x()+pos.x(), 'y':p.y()+pos.y(), 't':p.t()+pos.t(), 'l':p.l()+pos.l(), 'c':c} 
             for p in m.gen_piece_move(pos, c)]
    boards_data = convert_boards_data(m.get_boards())
    host.show(boards_data, highlights=[
        {
            'color':'#ff8080',
            'coordinates':[{'x':pos.x(), 'y':pos.y(), 't':pos.t(), 'l':pos.l(), 'c':c}]
        },
        {
            'color': '#80ff80',
            'coordinates': moves
        }
    ])
