#include <array>
int continuous(const std::array<std::array<int, 5>, 5>& board)
{

    std::array<std::array<bool, 5>, 5> visited = {};
    auto dfs = [&](int x, int y, int n, const auto& recurse) {
        if (x < 0 || x >= 5 || y < 0 || y >= 5 || board[y][x] != n || visited[y][x]) {
            return 0;
        }
        visited[y][x] = true;
        return recurse(x + 1, y, n, recurse) + recurse(x - 1, y, n, recurse) + recurse(x, y + 1, n, recurse) + recurse(x, y - 1, n, recurse) + 1;
    };
    int attacker_score = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (int area = dfs(x, y, board[y][x], dfs); area != 0) {
                attacker_score += board[y][x] + 5;
            }
        }
    }
    return attacker_score;
}
#include "full_game.hpp"
#include "game.hpp"

#include "negaalpha.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

#ifndef THRESHOLD
#define THRESHOLD 0
#endif

int main()
{

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution distri;

    int player;
    std::cin >> player;
    std::cout << "2 2" << std::endl;

    for (int i = 0; i <= 1000; i++) {
        int turn, timeleft, score_of_turn_player, score_of_other_player;
        std::cin >> turn >> timeleft >> score_of_turn_player >> score_of_other_player;
        Game2048::ReducerState my_state;
        std::cin >> my_state;
        Game2048::AttackerStateMulti opponent_state;
        std::cin >> opponent_state;

        int my_count = 0, opponent_count = 0;
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (my_state.data[y][x] != 0)
                    my_count++;
                if (opponent_state.data[y][x] != 0)
                    opponent_count++;
            }
        }
        std::cerr << "TURN " << turn << "\n";
        constexpr int threshold = THRESHOLD;

        if (my_count >= threshold || opponent_count >= threshold) {
            Game2048Full::State<0, true> full_state = {my_state.data, opponent_state.data};
            Result<Game2048Full::State<0, true>> res = {};
            using namespace std::chrono_literals;
            auto start_time = std::chrono::steady_clock::now();
            for (int depth = 1; depth <= 20; depth += 1) {
                try {

                    res = iterativeDeepeningNegaalpha(full_state, Game2048Full::eval, depth, depth - 1, [&] { return std::chrono::steady_clock::now() - start_time > 200ms; });
                    //                                        res = iterativeDeepeningNegaalpha(full_state, Game2048Full::eval, depth, depth - 1, [&] { return false; });

                    if (std::abs(res.score) == INF)
                        break;

                } catch (Abort&) {

                    //                    std::cerr << "Aborted" << std::endl;
                    break;
                }
            }

            auto& [depth, score, move] = res;

            if (depth == 0) {
                try {
                    forEachMove(full_state, [&res](const auto& m, const auto&) {
                        res.move = m;
                        throw Abort{};
                    });
                } catch (Abort&) {
                }
            }
            std::cerr << "Info " << depth << " " << score << " " << move << std::endl;

            std::cout << "RULD"[move.dir] << ' ' << move.points.size() << ' ' << move.n;
            for (auto& p : move.points) {
                std::cout << ' ' << p.y + 1 << ' ' << p.x + 1;
            }
            std::cout << std::endl;
        } else {

            auto start_time = std::chrono::steady_clock::now();
            {
                Result<Game2048::ReducerState> res = {};
                using namespace std::chrono_literals;

                for (int depth = 1; depth <= 15; depth += 2) {
                    try {
                        res = iterativeDeepeningNegaalpha(my_state, Game2048::eval, depth, depth - 2, [&] { return std::chrono::steady_clock::now() - start_time > 100ms; });
                        //                                                res = iterativeDeepeningNegaalpha(my_state, Game2048::eval, depth, depth - 2, [&] { return false; });

                    } catch (Abort&) {

                        break;
                    }
                }

                auto& [depth, score, move] = res;
                if (depth == 0) {
                    try {
                        forEachMove(my_state, [&res](const auto& m, const auto&) {
                            res.move = m;
                            throw Abort{};
                        });
                    } catch (Abort&) {
                    }
                }
                std::cerr << "Reducer: " << depth << " " << score << " " << move << std::endl;
                std::cout << "RULD"[move.dir] << ' ';

                opponent_state.n = my_state.countReduction(move) + 1;
            }
            {
                Result<Game2048::AttackerStateMulti> res = {};
                using namespace std::chrono_literals;

                for (int depth = 2; depth <= 20; depth += 2) {
                    try {
                        res = iterativeDeepeningNegaalpha(opponent_state, Game2048::eval, depth, depth - 2, [&] { return std::chrono::steady_clock::now() - start_time > 200ms; });
                        //                                                res = iterativeDeepeningNegaalpha(opponent_state, Game2048::eval, depth, depth - 2, [&] { return false; });

                    } catch (Abort&) {
                        break;
                    }
                }

                auto& [depth, score, move] = res;
                if (depth == 0) {
                    try {
                        forEachMove(opponent_state, [&res](const auto& m, const auto&) {
                            res.move = m;
                            throw Abort{};
                        });
                    } catch (Abort&) {
                    }
                }
                std::cerr << "Attacker: " << depth << " " << score << " " << move << std::endl;
                std::cout << move.points.size() << ' ' << move.n;
                for (auto& p : move.points) {
                    std::cout << ' ' << p.y + 1 << ' ' << p.x + 1;
                }
                std::cout << std::endl;
            }
        }
    }
    return 0;
}