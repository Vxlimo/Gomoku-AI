#include "AIController.h"
#include <cstring>
#include <random>
#include <unordered_map>
#include <utility>
#define ull unsigned long long

extern int ai_side; // 0: black, 1: white
std::string ai_name = "mcts";

int turn = 0;
int board[15][15];
int dx[4] = { 1, 0, 1, 1 }, dy[4] = { 0, 1, 1, -1 };
ull board_hash[15][15][2];
ull cur_state;
std::unordered_map<ull, std::pair<int, int>> hash_map;
const int TRAINING_TIMES = 50000;
std::mt19937 mt_rand(time(0));

// init function is called once at the beginning
void init()
{
    /* TODO: Replace this by your code */
    memset(board, -1, sizeof(board));
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            board_hash[x][y][0] = 1ll * mt_rand() * mt_rand() + 1;
            board_hash[x][y][1] = 1ll * mt_rand() * mt_rand() + 1;
        }
    }
}

bool check_win(int x, int y, int side)
{
    for (int i = 0; i < 4; i++) {
        int cnt = 1;
        for (int j = 1; j <= 4; j++) {
            int nx = x + dx[i] * j;
            int ny = y + dy[i] * j;
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15 || board[nx][ny] != side)
                break;
            cnt++;
        }
        for (int j = 1; j <= 4; j++) {
            int nx = x - dx[i] * j;
            int ny = y - dy[i] * j;
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15 || board[nx][ny] != side)
                break;
            cnt++;
        }
        if (cnt >= 5)
            return true;
    }
    return false;
}
bool full()
{
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] == -1)
                return false;
        }
    }
    return true;
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.

bool check_near(std::pair<int, int> pos)
{
    auto [x, y] = pos;
    for (int dir = 0; dir < 4; dir++) {
        for (int step = -1; step <= 1; step++) {
            int nx = x + dx[dir] * step;
            int ny = y + dy[dir] * step;
            if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15)
                continue;
            if (board[nx][ny] != -1) {
                return true;
            }
        }
    }
    return false;
}

std::pair<int, int> getRandom()
{
    std::vector<std::pair<int, int>> possible;
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] != -1)
                continue;
            if (check_near({ x, y }))
                possible.push_back({ x, y });
        }
    }
    if (possible.size() == 0) {
        possible.push_back({ 7, 7 });
    }
    int pos = mt_rand() % possible.size();
    return possible[pos];
}

double UCB(ull state, ull new_state, int side)
{
    if (hash_map.find(new_state) == hash_map.end())
        return -1e18;
    auto [score, times] = hash_map[state];
    auto [new_score, new_times] = hash_map[new_state];
    return (side == 0 ? 1.0 : -1.0) * new_score / (new_times + 1) + sqrt(1.96 * log(times) / (new_times + 1));
}

int simulating(int side)
{
    if (full())
        return 0;
    std::pair<int, int> pos = getRandom();
    board[pos.first][pos.second] = side;
    if (check_win(pos.first, pos.second, side)) {
        board[pos.first][pos.second] = -1;
        return side == 0 ? 1 : -1;
    }
    int score = simulating(1 - side);
    board[pos.first][pos.second] = -1;
    return score;
}

std::pair<std::pair<int, int>, int> training(ull state, int side)
{
    if (full())
        return { { -1, -1 }, 0 };
    std::vector<std::pair<int, int>> sons;
    for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
            if (board[x][y] == -1 && check_near({ x, y })) {
                ull new_state = state ^ board_hash[x][y][side];
                sons.push_back({ x, y });
            }
        }
    }
    std::pair<int, int> best_pos = sons[0];
    double best_ucb = -1e18;
    for (auto [x, y] : sons) {
        ull new_state = state ^ board_hash[x][y][side];
        if (hash_map.find(new_state) == hash_map.end()) {
            hash_map[new_state] = { 0, 0 };
        }
        double ucb = UCB(state, new_state, side);
        if (ucb > best_ucb) {
            best_ucb = ucb;
            best_pos = { x, y };
        }
    }
    ull new_state = state ^ board_hash[best_pos.first][best_pos.second][side];
    if (check_win(best_pos.first, best_pos.second, side)) {
        hash_map[new_state].first += side == 0 ? 1 : -1;
        hash_map[new_state].second++;
        return { best_pos, side == 0 ? 1 : -1 };
    }
    board[best_pos.first][best_pos.second] = side;
    int score;
    if (hash_map[new_state].second) {
        score = training(new_state, 1 - side).second;
        hash_map[new_state].first += score;
        hash_map[new_state].second++;
    } else {
        score = simulating(1 - side);
        hash_map[new_state].first += score;
        hash_map[new_state].second++;
    }
    board[best_pos.first][best_pos.second] = -1;
    return { best_pos, score };
}

std::pair<int, int> action(std::pair<int, int> loc)
{
    /* TODO: Replace this by your code */
    /* This is now a random strategy */

    turn++;
    if (loc.first == -1 && loc.second == -1) {
        ai_side = 1 - ai_side;
    } else {
        board[loc.first][loc.second] = 1 - ai_side;
        cur_state ^= board_hash[loc.first][loc.second][1 - ai_side];
    }
    // if (turn == 2 && ai_side == 1) {

    // } else {
    hash_map.clear();
    for (int i = 0; i < TRAINING_TIMES; i++) {
        int score = training(cur_state, ai_side).second;
        hash_map[cur_state].first += score;
        hash_map[cur_state].second++;
    }
    auto [pos, score] = training(cur_state, ai_side);
    // std::cerr << score << std::endl;
    board[pos.first][pos.second] = ai_side;
    cur_state ^= board_hash[pos.first][pos.second][ai_side];
    // }
    return pos;
}