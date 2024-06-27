#include "AIController.h"
#include <cstring>
#include <tuple>
#include <unordered_map>
#include <utility>
extern int ai_side; // 0: black, 1: white
std::string ai_name = "no_name";

int turn = 0;
int board[15][15], state;
int state_hash[15][15][3];
int side_hash;
std::unordered_map<int, int> state_score;
int dis_score[15][15];
int line_score[2187];

// init function is called once at the beginning
void init()
{
    /* TODO: Replace this by your code */
    srand(time(0));
    memset(board, -1, sizeof(board));
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++)
            dis_score[x][y] = std::min(std::min(x, 14 - x), std::min(y, 14 - y));
    }
    int state[7];
    for (int i = 0; i < 2187; i++) {
        for (int j = 6, k = i; j >= 0; j--) {
            state[j] = k % 3;
            k /= 3;
        }
        if (state[0] == 1 || state[6] == 1)
            continue;
        int l = 1, r = 1;
        bool l_open = false, r_open = false;
        while (r < 6) {
            while (state[l] != 1 && l < 6)
                l++;
            if (l >= 6)
                break;
            r = l;
            while (state[r + 1] == 1 && r + 1 < 6)
                r++;
            l_open = state[l - 1] == 0;
            r_open = state[r + 1] == 0;
            if (r - l + 1 >= 5) {
                line_score[i] += 1e8;
            }
            if (r - l + 1 == 4) {
                if (l_open && r_open)
                    line_score[i] += 1e5;
                else {
                    if (l_open || r_open)
                        line_score[i] += 1e4;
                }
            }
            if (r - l + 1 == 3) {
                if (l_open && r_open)
                    line_score[i] += 1e3;
                else {
                    if (l_open || r_open)
                        line_score[i] += 1e2;
                }
            }
            if (r - l + 1 == 2) {
                if (l_open && r_open)
                    line_score[i] += 1e2;
                else {
                    if (l_open || r_open)
                        line_score[i] += 10;
                }
            }
            l = r + 1;
        }
    }
    side_hash = rand();
    for (int x = 0; x < 15; x++)
        for (int y = 0; y < 15; y++) {
            for (int i = 0; i < 3; i++)
                state_hash[x][y][i] = rand();
        }
    return;
}

void calc(int x, int y, int side, int& hash_0, int& hash_1)
{
    if (board[x][y] == -1) {
        hash_0 = hash_0 * 3;
        hash_1 = hash_1 * 3;
    }
    if (board[x][y] == side) {
        hash_0 = hash_0 * 3 + 1;
        hash_1 = hash_1 * 3 + 2;
    }
    if (board[x][y] == (side ^ 1)) {
        hash_0 = hash_0 * 3 + 2;
        hash_1 = hash_1 * 3 + 1;
    }
    hash_0 %= 2187;
    hash_1 %= 2187;
    return;
}
int score_board(int side)
{
    if (state_score.count(state))
        return state_score[state];
    int my_score = 0, op_score = 0;
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] == side)
                my_score += dis_score[x][y];
            if (board[x][y] == (side ^ 1))
                op_score += dis_score[x][y];
        }
    }
    for (int x = 0; x < 15; x++) {
        int hash_0 = 2, hash_1 = 2;
        for (int y = 0; y < 15; y++) {
            calc(x, y, side, hash_0, hash_1);
            if (y >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
        }
    }
    for (int y = 0; y < 15; y++) {
        int hash_0 = 2, hash_1 = 2;
        for (int x = 0; x < 15; x++) {
            calc(x, y, side, hash_0, hash_1);
            if (x >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
        }
    }
    for (int i = 0; i < 15; i++) {
        int x = i, y = 0;
        int hash_0 = 2, hash_1 = 2;
        while (x < 15 && y < 15) {
            calc(x, y, side, hash_0, hash_1);
            if (x - i >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
            x++;
            y++;
        }
        x = 0, y = i;
        hash_0 = 2, hash_1 = 2;
        while (x < 15 && y < 15) {
            calc(x, y, side, hash_0, hash_1);
            if (x >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
            x++;
            y++;
        }
    }
    for (int i = 0; i < 15; i++) {
        int x = i, y = 14;
        int hash_0 = 2, hash_1 = 2;
        while (x < 15 && y >= 0) {
            calc(x, y, side, hash_0, hash_1);
            if (x - i >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
            x++;
            y--;
        }
        x = 0, y = i;
        hash_0 = 2, hash_1 = 2;
        while (x < 15 && y >= 0) {
            calc(x, y, side, hash_0, hash_1);
            if (x >= 5) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
            }
            x++;
            y--;
        }
    }
    state_score[state] = my_score - op_score;
    return my_score - op_score;
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.

std::pair<int, int> getRandom()
{
    while (true) {
        int x = rand() % 15;
        int y = rand() % 15;
        if (board[x][y] == -1) {
            board[x][y] = ai_side;
            return std::make_pair(x, y);
        }
    }
}

std::pair<std::pair<int, int>, int> search(int depth, int side, int alpha, int beta)
{
    if (depth == 0) {
        return std::make_pair(std::make_pair(-1, -1), score_board(side));
    }
    std::pair<int, int> best_pos = std::make_pair(-1, -1);
    int best_score = -1e9;
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] != -1) {
                continue;
            }
            board[x][y] = side;
            state ^= state_hash[x][y][0] ^ state_hash[x][y][side + 1];
            state ^= side_hash;
            auto [pos, score] = search(depth - 1, side ^ 1, -alpha - 1, -alpha);
            if (score <= alpha)
                std::tie(pos, score) = search(depth - 1, side ^ 1, -beta, -alpha);
            board[x][y] = -1;
            state ^= state_hash[x][y][0] ^ state_hash[x][y][side + 1];
            state ^= side_hash;
            score = -score;
            if (score > best_score) {
                best_score = score;
                best_pos = std::make_pair(x, y);
            }
            alpha = std::max(alpha, best_score);
            if (alpha >= beta) {
                return std::make_pair(best_pos, best_score);
            }
        }
    }
    return std::make_pair(best_pos, best_score);
}

std::pair<int, int> action(std::pair<int, int> loc)
{
    /* TODO: Replace this by your code */
    /* This is now a random strategy */

    turn++;
    if (loc.first == -1 && loc.second == -1) {
        if (turn != 1)
            ai_side ^= 1;
    } else {
        board[loc.first][loc.second] = ai_side ^ 1;
        state ^= state_hash[loc.first][loc.second][0] ^ state_hash[loc.first][loc.second][(ai_side ^ 1) + 1];
    }
    std::cerr << score_board(ai_side) << " " << state << std::endl;
    auto [pos, score] = search(4, ai_side, -1e9, 1e9);
    board[pos.first][pos.second] = ai_side;
    state ^= state_hash[pos.first][pos.second][0] ^ state_hash[pos.first][pos.second][ai_side + 1];
    return pos;
}