from build import engine

g = engine.game("""
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]
                """)
fm = engine.full_move.move(engine.vec4(0,1,1,0),engine.vec4(0,1,0,0))

print(g.apply_move(fm))

print(g.get_current_boards())


