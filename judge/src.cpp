#include "AIController.h"
#include <cstring>
#include <queue>
#include <tuple>
#include <utility>
#define ll long long

extern int ai_side;
std::string ai_name = "no_name";

int turn = 0;
int board[15][15];
int dis_score[15][15];
ll line_score[729], cur_score;
int dx[4] = { 1, 0, 1, 1 }, dy[4] = { 0, 1, 1, -1 };
const int SEARCH_DEPTH = 6;
const int SEARCH_WIDTH = 20;
const int KILLER_SEARCH_DEPTH = 10;

void init()
{
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            board[x][y] = -1;
            dis_score[x][y] = std::min(std::min(x, 14 - x), std::min(y, 14 - y));
        }
    }
    int line[6];
    for (int i = 0; i < 729; i++) {
        for (int j = 5, k = i; j >= 0; j--) {
            line[j] = k % 3;
            k /= 3;
        }
        if (line[1] != 1)
            continue;
        int l = 1, r = 1, cnt = 1, last = 1;
        bool l_open = false, r_open = false;
        while (r + 1 <= 5 && line[r + 1] != 2) {
            if (line[r + 1] == 1) {
                cnt++;
                last = r + 1;
            }
            r++;
        }
        l_open = line[l - 1] == 0;
        if (last == 5)
            r_open = false;
        else
            r_open = line[last + 1] == 0;
        if (cnt >= 5) {
            line_score[i] += 1e15;
        }
        if (cnt == 4) {
            if (l_open && r_open && last == 4)
                line_score[i] += 1e10;
            else {
                if (l_open || r_open)
                    line_score[i] += 1e6;
                else {
                    if (last == 5)
                        line_score[i] += 1e6 - 1e5;
                }
            }
        }
        if (cnt == 3) {
            if (l_open && r_open && last <= 4)
                line_score[i] += 1e6;
            else {
                if (l_open || r_open || last <= 5)
                    line_score[i] += 1e4;
            }
        }
        if (cnt == 2) {
            if (l_open && r_open && last <= 3)
                line_score[i] += 1e4;
            else {
                if (l_open || r_open)
                    line_score[i] += 1e2;
            }
        }
    }
    return;
}

void calc(int x, int y, int side, int& hash_0, int& hash_1)
{
    hash_0 = hash_0 * 3;
    hash_1 = hash_1 * 3;
    if (x < 0 || x >= 15 || y < 0 || y >= 15) {
        hash_0 = hash_0 + 2;
        hash_1 = hash_1 + 2;
    } else {
        if (board[x][y] == side) {
            hash_0 = hash_0 + 1;
            hash_1 = hash_1 + 2;
        }
        if (board[x][y] == (1 - side)) {
            hash_0 = hash_0 + 2;
            hash_1 = hash_1 + 1;
        }
    }
    hash_0 %= 729;
    hash_1 %= 729;
    return;
}
ll score_point(int x, int y, int side)
{
    ll my_score = 0, op_score = 0;
    int my_win = 0, op_win = 0;
    for (int dir = 0; dir < 4; dir++) {
        int nx, ny;
        int hash_0 = 0, hash_1 = 0;
        for (int i = -5; i <= 5; i++) {
            nx = x + i * dx[dir];
            ny = y + i * dy[dir];
            calc(nx, ny, side, hash_0, hash_1);
            if (i >= 0) {
                my_score -= line_score[hash_0];
                op_score -= line_score[hash_1];
                if (line_score[hash_0] == 1e15) {
                    my_win = 1;
                }
                if (line_score[hash_1] == 1e15)
                    op_win = 1;
            }
        }
    }
    if (my_win)
        return 1e17;
    if (op_win)
        return -1e17;
    board[x][y] = side;
    for (int dir = 0; dir < 4; dir++) {
        int nx, ny;
        int hash_0 = 0, hash_1 = 0;
        for (int i = -5; i <= 5; i++) {
            nx = x + i * dx[dir];
            ny = y + i * dy[dir];
            calc(nx, ny, side, hash_0, hash_1);
            if (i >= 0) {
                my_score += line_score[hash_0];
                op_score += line_score[hash_1];
                if (line_score[hash_0] == 1e15) {
                    my_win = 1;
                }
                if (line_score[hash_1] == 1e15)
                    op_win = 1;
            }
        }
    }
    my_score += dis_score[x][y];
    board[x][y] = -1;
    if (my_win)
        return 1e17;
    if (op_win)
        return -1e17;
    return my_score - op_score;
}

bool check_near(int x, int y)
{
    for (int i = x - 2; i <= x + 2; i++) {
        for (int j = y - 2; j <= y + 2; j++) {
            if (i < 0 || i >= 15 || j < 0 || j >= 15)
                continue;
            if (board[i][j] != -1)
                return true;
        }
    }
    return false;
}
std::pair<std::pair<int, int>, ll> search(int cur_turn, int depth, int side, ll alpha, ll beta, ll score, bool killer = false)
{
    if (depth == 0) {
        return std::make_pair(std::make_pair(-1, -1), score);
    }
    std::pair<int, int> best_pos = std::make_pair(-1, -1);
    ll best_score = -1e18;
    std::priority_queue<std::pair<ll, std::pair<int, int>>> pos_list;
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] == -1 && (cur_turn <= 2 || check_near(x, y))) {
                pos_list.push(std::make_pair(score_point(x, y, side), std::make_pair(x, y)));
            }
        }
    }
    if (cur_turn == 2 && side == 1) {
        pos_list.push(std::make_pair(-2 * score, std::make_pair(-1, -1)));
    }
    for (int i = 1; i <= std::min(SEARCH_WIDTH, (int)pos_list.size()); i++) {
        int x = pos_list.top().second.first, y = pos_list.top().second.second;
        ll delta = pos_list.top().first;
        pos_list.pop();
        if (delta >= 1e15) {
            return std::make_pair(std::make_pair(x, y), delta);
        }
        if (killer && delta <= 1e6)
            break;
        std::pair<int, int> pos;
        ll new_score;
        if (x == -1 && y == -1) {
            std::tie(pos, new_score) = search(3, depth - 1, side, -beta, -alpha, -(score + delta), killer);
            new_score = -new_score;
        } else {
            board[x][y] = side;
            if (i != 1) {
                std::tie(pos, new_score) = search(cur_turn + (side == 1), depth - 1, 1 - side, -alpha - 1, -alpha, -(score + delta), killer);
                new_score = -new_score;
            }
            if (i == 1 || (new_score > alpha && new_score < beta)) {
                std::tie(pos, new_score) = search(cur_turn + (side == 1), depth - 1, 1 - side, -beta, -alpha, -(score + delta), killer);
                new_score = -new_score;
            }
            board[x][y] = -1;
        }
        if (new_score > best_score || i == 1) {
            best_score = new_score;
            best_pos = std::make_pair(x, y);
        }
        alpha = std::max(alpha, best_score);
        if (alpha >= beta) {
            return std::make_pair(best_pos, best_score);
        }
    }
    return std::make_pair(best_pos, best_score);
}

std::pair<int, int> action(std::pair<int, int> loc)
{
    turn++;
    if (loc.first == -1 && loc.second == -1) {
        if (turn != 1)
            ai_side = 1 - ai_side;
    } else {
        cur_score -= score_point(loc.first, loc.second, 1 - ai_side);
        board[loc.first][loc.second] = 1 - ai_side;
    }
    auto [pos, score] = search(turn, KILLER_SEARCH_DEPTH, ai_side, -1e17, 1e17, cur_score, true);
    if (score < 1e15) {
        std::tie(pos, score) = search(turn, SEARCH_DEPTH, ai_side, -1e17, 1e17, cur_score);
    }
    if (pos.first != -1 && pos.second != -1) {
        cur_score += score_point(pos.first, pos.second, ai_side);
        board[pos.first][pos.second] = ai_side;
    } else {
        ai_side = 1 - ai_side;
        cur_score = -cur_score;
    }
    return pos;
}