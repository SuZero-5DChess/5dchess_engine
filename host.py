from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit
import sys, os
original_sys_path = sys.path.copy()
try:
    sys.path.append(os.path.join(os.path.dirname(__file__), 'build'))
    import engine # type: ignore
finally:
    sys.path = original_sys_path

base_dir = os.path.abspath('extern/client')
static_dir = os.path.join(base_dir, 'static')
template_dir = os.path.join(base_dir, 'templates')

app = Flask(__name__, static_folder=static_dir, template_folder=template_dir)
socketio = SocketIO(app)

t0_fen = """
[Size "8x8"]
[Board "custom"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]
"""
tminf_fen = """
[Size "8x8"]
[Board "custom"]
""" + '\n'.join([f'[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:{i}:b][r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:{i+1}:w]' for i in range(0, 41)])
very_small_open = """
[Size "4x4"]
[Board "custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]
"""
g = engine.game(t0_fen)
game_data = {}


@app.route('/')
def index():
    return render_template('index.html')

qs = []
p0 = engine.vec4(0,0,0,0)

@socketio.on('click')
def handle_click(data):
    l = int(data['l'])
    t = int(data['t'])
    c = int(data['c'])
    x = int(data['x'])
    y = int(data['y'])

    c1 = "wb"[data['c']]
    x1 = chr(data['x']+ord('a'))
    y1 = chr(data['y']+ord('1'))
    print(f"Received mouse click: ({l}T{t}{c1}){x1}{y1}")

    global qs, p0, g
    pos = engine.vec4(x,y,t,l)
    present_t, present_c = g.get_current_present()
    print(pos, qs, pos in qs)
    if pos in qs:
        fm = engine.move5d.move(p0, pos)
        flag = g.apply_move(fm)
        hl = []
        if flag:
            if g.currently_check():
                checks = g.get_current_checks()
                arrows = []
                for p, q in checks:
                    arrows.append({
                        'from': {'l':p.l(), 't': p.t(), 'x':p.x(), 'y':p.y(), 'c':1-present_c},
                        'to': {'l':q.l(), 't': q.t(), 'x':q.x(), 'y':q.y(), 'c':1-present_c},
                    })
                    print('piece on', p, 'is checking', q)
                hl = [
                    {
                        'color':'#ff1111',
                        'arrows':arrows
                    },
                ]
                print('applying move ', fm, ' --success (checking)')
            else:
                print('applying move ', fm, ' --success')
        else:
            print('applying move ', fm, ' --failure')
        qs = []
        display(hl)
    elif c == present_c:
        qs = g.gen_move_if_playable(pos)
        #print('ds = ', ds)
        moves = [{'x':q.x(), 'y':q.y(), 't':q.t(), 'l':q.l(), 'c':present_c} for q in qs]
        hl = [
            {
                'color':'#ff80c0',
                'coordinates':[{'x':pos.x(), 'y':pos.y(), 't':pos.t(), 'l':pos.l(), 'c':c}]
            },
            {
                'color': '#80ff80',
                'coordinates': moves
            }
        ]
        p0 = pos
        display(hl)
    else:
        print('no piece at click')
        qs = []
        display()

@socketio.on('right_click')
def handle_click(data):
    print('canceled click')
    global qs
    qs = []
    display()

@socketio.on('request_undo')
def handle_undo():
    print('attempting undo', end='')
    flag = g.undo()
    print(' ---', 'success' if flag else 'failed')
    display()

@socketio.on('request_redo')
def handle_redo():
    print('attempting redo', end='')
    flag = g.redo()
    print(' ---', 'success' if flag else 'failed')
    display()

@socketio.on('request_submit')
def handle_submit():
    print('received submition request', end='')
    flag = g.apply_move(engine.move5d.submit())
    print(' ---', 'success' if flag else 'failed')
    display()

def convert_boards_data(boards):
    def convert_board(board):
        l, t, c, s = board
        return {"l":l, "t":t, "c":c, "fen":s}
    return list(map(convert_board, boards))

def display(hl=[]):
    mandatory, optional, unplayable = g.get_current_timeline_status()
    present_t, present_c = g.get_current_present()
    critical = g.get_movable_pieces()
    cc = [{'x':q.x(), 'y':q.y(), 't':q.t(), 'l':q.l(), 'c':present_c} for q in critical]
    match_status = g.get_match_status()
    text = f"""
<p>{'white' if present_c == 0 else 'black'}'s move</p>
<p>{str(match_status)}</p>
"""[1:-1]
    is_game_over = match_status != engine.match_status_t.PLAYING
    emit('response_text', text)
    size_x, size_y = g.get_board_size()
    new_data = {
        'submit-button': 'enabled' if g.can_submit() else 'disabled',
        'undo-button': 'enabled' if g.can_undo() else 'disabled',
        'redo-button': 'enabled' if g.can_redo() else 'disabled',
        'metadata': {
            "mode" : "odd"
        },
        'size': {
            'x':size_x,
            'y':size_y
        },
        'present': {
            't': present_t,
            'c': present_c,
            'color': 'rgba(219,172,52,0.4)' if not is_game_over else 'rgba(128,128,128,0.4)'
        },
        'focus': [
            {
                'l': line,
                't': present_t,
                'c': present_c
            } for line in mandatory
        ],
        'boards':convert_boards_data(g.get_current_boards()),
        'highlights':[
            {
                'color':'#7070ff',
                'timelines': mandatory,
            },
            {
                'color':'#80ff80',
                'timelines': optional,
            },
            {
                'color':'#ffaaaa',
                'timelines': unplayable,
            },
            {
                'color':'#a569bd',
                'coordinates': cc,
            }
        ] + hl
    }
    game_data.update(new_data)
    #print('displaying ', g.get_current_boards())
    emit('response_data', game_data)

@socketio.on('request_data')
def handle_request(data):
    display()

if __name__ == '__main__':
    socketio.run(app, debug=True)
