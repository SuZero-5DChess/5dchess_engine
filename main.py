from build import engine

fen = """
[Size:"8x8"]
[Board:"custom"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]
"""

if __name__ == '__main__':
    board = engine.board5d(fen)
    b = board.get_board(0,1,0)
    print(board)
