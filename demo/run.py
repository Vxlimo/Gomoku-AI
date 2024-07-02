from flask import Flask, request, jsonify
from flask_cors import CORS
import random

app = Flask(__name__)
CORS(app)  # 允许所有来源的跨域请求

# 初始化一个空的15x15棋盘
board = [['' for _ in range(15)] for _ in range(15)]

def check_winner(board, player):
    # 简单的胜利检查逻辑，需完善
    return False

@app.route('/move', methods=['POST'])
def move():
    data = request.get_json()
    row = data['row']
    col = data['col']
    player = data['player']
    board[row][col] = player

    if check_winner(board, player):
        return jsonify(status='success', message='Move accepted', winner=player)

    # AI的简单决策逻辑，随机选择一个空位置
    empty_cells = [(r, c) for r in range(15) for c in range(15) if board[r][c] == '']
    if empty_cells:
        ai_move = random.choice(empty_cells)
        board[ai_move[0]][ai_move[1]] = 'O'
        if check_winner(board, 'O'):
            return jsonify(status='success', message='Move accepted', winner='O', aiMove={'row': ai_move[0], 'col': ai_move[1]})
        return jsonify(status='success', message='Move accepted', aiMove={'row': ai_move[0], 'col': ai_move[1]})

    return jsonify(status='success', message='Move accepted', winner=None, aiMove=None)

if __name__ == '__main__':
    app.run(debug=True)
