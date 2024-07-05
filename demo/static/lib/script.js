const boardSize = 15;
let board = [];
let isPlayerTurn = false;
let gameActive = false;
let playerSide;
var turn = 0;

function createBoard() {
    const gameBoard = document.getElementById('game-board');
    for (let i = 0; i < boardSize; i++) {
        for (let j = 0; j < boardSize; j++) {
            const cell = document.createElement('div');
            cell.classList.add('cell');
            cell.dataset.row = i;
            cell.dataset.col = j;
            cell.addEventListener('click', handleMove);
            gameBoard.appendChild(cell);
        }
    }
}

function resetBoard() {
    board = Array.from({ length: boardSize }, () => Array(boardSize).fill(''));
    const cells = document.querySelectorAll('.cell');
    cells.forEach(cell => cell.textContent = '');
    isPlayerTurn = gameActive = false;
    turn = 0;
    document.getElementById('player-first').disabled = false;
    document.getElementById('player-second').disabled = false;
    document.getElementById('change').disabled = true;
    document.getElementById('turn').textContent = '';
    document.getElementById('special').textContent = 'Choose your side';
}

function startGame(side) {
    playerSide = side;
    gameActive = true;
    document.getElementById('player-first').disabled = true;
    document.getElementById('player-second').disabled = true;
    if (playerSide === 0)
        document.getElementById('turn').textContent = 'Your turn';
    else
        document.getElementById('turn').textContent = 'AI\'s turn';
    document.getElementById('special').textContent = '';

    fetch('/start', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ ai_side: playerSide })
    })
        .then(response => response.json())
        .then(data => {
            if (playerSide === 1) {
                isPlayerTurn = false;
                sendMove([-1, -1]);
            } else {
                isPlayerTurn = true;
            }
        });
}

function handleMove(event) {
    if (!isPlayerTurn || !gameActive)
        return;
    const row = parseInt(event.target.dataset.row);
    const col = parseInt(event.target.dataset.col);
    if (board[row][col] !== '')
        return;

    turn++;
    board[row][col] = 'X';
    event.target.textContent = 'X';
    isPlayerTurn = false;
    document.getElementById('turn').textContent = 'AI\'s turn';
    document.getElementById('special').textContent = '';
    document.getElementById('change').disabled = true;
    if (checkWin(row, col, 'X')) {
        endGame('You win!');
    }

    sendMove([row, col]);
}

function sendMove(move) {
    fetch('/move', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ move: move })
    })
        .then(response => response.json())
        .then(data => {
            const row = data.move[0];
            const col = data.move[1];
            if (row === -1 && col === -1) {
                flipBoard();
                isPlayerTurn = true;
                document.getElementById('turn').textContent = 'Your turn';
                document.getElementById('special').textContent = 'AI flipped the board';
            } else {
                turn++;
                board[row][col] = 'O';
                const cell = document.querySelector(`.cell[data-row='${row}'][data-col='${col}']`);
                cell.textContent = 'O';
                isPlayerTurn = true;
                document.getElementById('turn').textContent = 'Your turn';
                document.getElementById('special').textContent = '';
                if (turn == 3) {
                    document.getElementById('change').disabled = false;
                    document.getElementById('special').textContent = 'You can flip the board now';
                }
                if (checkWin(row, col, 'O')) {
                    endGame('AI wins!');
                } else if (isBoardFull()) {
                    endGame('It\'s a draw!');
                }
            }
        });
}

function flipBoard() {
    document.getElementById('change').disabled = true;
    turn++;
    for (let i = 0; i < boardSize; i++) {
        for (let j = 0; j < boardSize; j++) {
            if (board[i][j] === 'X') {
                board[i][j] = 'O';
            } else if (board[i][j] === 'O') {
                board[i][j] = 'X';
            }
            const cell = document.querySelector(`.cell[data-row='${i}'][data-col='${j}']`);
            cell.textContent = board[i][j];
        }
    }
    if (isPlayerTurn) {
        document.getElementById('turn').textContent = 'AI\'s turn';
        document.getElementById('special').textContent = '';
        sendMove([-1, -1]);
    }
}

function checkWin(row, col, player) {
    const directions = [
        { x: 1, y: 0 }, { x: 0, y: 1 },
        { x: 1, y: 1 }, { x: 1, y: -1 }
    ];
    for (const { x, y } of directions) {
        let count = 1;
        count += countDirection(row, col, player, x, y);
        count += countDirection(row, col, player, -x, -y);
        if (count >= 5) return true;
    }
    return false;
}

function countDirection(row, col, player, dx, dy) {
    let count = 0;
    for (let i = 1; i < 5; i++) {
        const r = row + i * dx;
        const c = col + i * dy;
        if (r < 0 || r >= boardSize || c < 0 || c >= boardSize || board[r][c] !== player) {
            break;
        }
        count++;
    }
    return count;
}

function isBoardFull() {
    return board.every(row => row.every(cell => cell !== ''));
}

function endGame(message) {
    gameActive = false;
    document.getElementById('turn').textContent = message;
    document.getElementById('special').textContent = '';
}

document.getElementById('player-first').addEventListener('click', () => startGame(0));
document.getElementById('player-second').addEventListener('click', () => startGame(1));
document.getElementById('reset').addEventListener('click', resetBoard);
document.getElementById('change').addEventListener('click', flipBoard);

createBoard();
document.getElementById('game-board').style.display = 'grid';
resetBoard();