from build import engine

fen = """
[Size "8x8"]
[Board  "custom"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]
"""

def convert_boards_data(boards):
    def convert_board(board):
        l, t, c, s = board
        return {"l":l, "t":t, "c":c, "fen":s}
    return list(map(convert_board, boards))

if __name__ == '__main__':
    m = engine.multiverse(fen)
    print(convert_boards_data(m.get_boards()))
