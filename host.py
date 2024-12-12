from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit
import os

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
