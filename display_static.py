import sys
import os
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit

original_sys_path = sys.path.copy()
try:
    # Add the 'build' directory to sys.path temporarily
    sys.path.append(os.path.join(os.path.dirname(__file__), 'build'))
    import engine # type: ignore
finally:
    sys.path = original_sys_path


base_dir = os.path.abspath('extern/client')
static_dir = os.path.join(base_dir, 'static')
template_dir = os.path.join(base_dir, 'templates')

app = Flask(__name__, static_folder=static_dir, template_folder=template_dir)
socketio = SocketIO(app)

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('click')
def handle_click(data):
    l = data['l']
    t = data['t']
    c = "wb"[data['c']]
    x = chr(data['x']+ord('a'))
    y = chr(data['y']+ord('1'))
    print(f"Received mouse click: ({l}T{t}{c}){x}{y}")

@socketio.on('request_data')
def handle_request(data):
    pass

def show(boards, *args, **options):
    options['boards'] = boards
    options.setdefault('focus', {'l':0,'t':1,'c':1})
    
    @socketio.on('request_data')
    def handle_request(data):
        emit('response_data', options)
    
    socketio.run(app, debug=True)


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
    pos = engine.vec4(4, 2, 3, 0)
    c = 0
    moves = [{'x':p.x()+pos.x(), 'y':p.y()+pos.y(), 't':p.t()+pos.t(), 'l':p.l()+pos.l(), 'c':c} 
             for p in m.gen_piece_move(pos, c)]
    boards_data = convert_boards_data(m.get_boards())
    show(boards_data, highlights=[
        {
            'color':'#ff8080',
            'coordinates':[{'x':pos.x(), 'y':pos.y(), 't':pos.t(), 'l':pos.l(), 'c':c}]
        },
        {
            'color': '#80ff80',
            'coordinates': moves
        },
        {
            'color': '#8080ff',
            'arrows': [
                {
                    'from': {'l':0,'t':1,'x':4,'y':1,'c':0},
                    'to': {'l':0,'t':2,'x':4,'y':3,'c':0}
                }
            ]
        }
    ])
