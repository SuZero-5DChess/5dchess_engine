from build import engine

if __name__ == '__main__':
    b = engine.board2d("r*nbqk*bnr* /p*p*p*p*p*p*p*p* /8/8/8/8/P*P*P*P*P*P*P*P* /R*NBQK*BNR*")
    b.set_piece(1,2,engine.BISHOP_W)
    print(b)
