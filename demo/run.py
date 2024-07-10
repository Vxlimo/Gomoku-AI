from flask import Flask, request, jsonify, render_template
import subprocess
import os
import signal

app = Flask(__name__)
ai_process = None

@app.route('/')
def index():
    return render_template('index.html')

def start_ai(ai_side):
    global ai_process
    if ai_process:
        os.killpg(os.getpgid(ai_process.pid), signal.SIGTERM)  # Kill previous AI process
    ai_process = subprocess.Popen(['./ai'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=os.setsid)
    ai_process.stdin.write(f'{ai_side}\n'.encode())
    ai_process.stdin.flush()

@app.route('/start', methods=['POST'])
def start_game():
    data = request.get_json()
    ai_side = data['ai_side']
    start_ai(ai_side)
    return jsonify({'status': 'AI started'})

@app.route('/move', methods=['POST'])
def move():
    data = request.get_json()
    move = data['move']
    ai_process.stdin.write(f'{move[0]} {move[1]}\n'.encode())
    ai_process.stdin.flush()
    ai_response = ai_process.stdout.readline().decode().strip().split()
    ai_move = [int(ai_response[0]), int(ai_response[1])]
    return jsonify({'move': ai_move})

if __name__ == '__main__':
    app.run()
