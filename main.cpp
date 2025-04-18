#include "pattern.h"
#include "bitboard.h"
#include "operation.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <array>
#include <fstream>
#include <cassert>
#include <cmath>
#include <memory>
using namespace std;

const int WEIGHT_SIZE = 0xffffff + 1;

class TupleNetwork {
   public:
    TupleNetwork(const int patterns[][6], const int group_sizes[], int gn)
        : fn(0), patterns(patterns) {
        for (int i = 0; i < gn; ++i) {
            fn += group_sizes[i];
        }
        weights = new float*[fn];
        int i = 0;
        for (int g = 0; g < gn; ++g) {
            float* weight = new float[WEIGHT_SIZE];
            for (int j = 0; j < group_sizes[g]; ++j) {
                weights[i++] = weight;
            }
        }
    }

    float value(board_t board) const {
        float value = 0;
        for (int i = 0; i < fn; ++i) {
            unsigned feature = getFeature(board, patterns[i]);
            value += weights[i][feature];
        }
        return value / fn;
    }

    void update(board_t board, float delta, float alpha) {
        for (int i = 0; i < fn; ++i) {
            unsigned feature = getFeature(board, patterns[i]);
            weights[i][feature] += alpha * delta;
        }
    }

    void saveWeights(const string& filename) {
        ofstream ofs(filename, ios::binary);
        for (int i = 0; i < fn; ++i) {
            ofs.write(reinterpret_cast<char*>(weights[i]), WEIGHT_SIZE * sizeof(float));
        }
        ofs.close();
    }

    void loadWeights(const string& filename) {
        ifstream ifs(filename, ios::binary);
        for (int i = 0; i < fn; ++i) {
            ifs.read(reinterpret_cast<char*>(weights[i]), WEIGHT_SIZE * sizeof(float));
        }
        ifs.close();
    }

   private:
    unsigned getFeature(const board_t board, const int pattern[6]) const {
        unsigned feature = 0;
        for (int i = 0; i < 6; ++i) {
            feature <<= 4;
            feature |= (board >> (4 * pattern[i])) & 0xf;
        }
        // assert(feature < WEIGHT_SIZE);
        return feature;
    }
    int fn;
    const int (*patterns)[6];
    int* groups;
    float** weights;
};

int find_best_action(TupleNetwork& tn, const board_t state) {
    float max_value = -numeric_limits<float>::max();
    int best_action = -1;

    board sim_env(state);
    bool legal_actions[4];
    sim_env.getLegalActions(legal_actions);
    for (int action = 0; action < 4; ++action) {
        if (legal_actions[action]) {
            sim_env.reset(state);
            int reward = sim_env.move(action);
            float value = reward + tn.value(sim_env.getState());
            // cout << action << ": " << reward << " " << value << endl;
            if (value >= max_value) {
                max_value = value;
                best_action = action;
            }
        }
    }
    assert(best_action != -1);
    return best_action;
}

int main() {
    GameSetting::init();
    TupleNetwork tn(patterns, groups, gn);
    // tn.loadWeights("weights.bin");
    const int num_episodes = 300000;
    float alpha = 0.1;
    vector<int> final_scores;
    int max_tile = 0;
    float max_avg_score = 0;
    for (int t = 0; t < num_episodes; ++t) {
        board env;
        bool done = false;
        int score = 0;
        vector<pair<board_t, board_t>> trajectory;
        while (!done) {
            // cout << "state " << hex << env.getState() << endl;
            board_t state = env.getState();
            int action = find_best_action(tn, state);
            auto [next_state, reward, is_done, afterstate] = env.step(action);
            // cout << "action " << action << endl;
            // cout << "reward " << reward << endl;
            // cout << "afterstate " << hex << afterstate << endl;
            // cout << "next_state " << hex << next_state << endl;
            trajectory.push_back({afterstate, next_state});
            score += reward;
            done = is_done;

            // find max tile
            for (int i = 0; i < 16; i++) {
                int val = (state >> (4 * i)) & 0xf;
                if (val > max_tile) {
                    max_tile = val;
                }
            }
        }
        final_scores.push_back(score);

        for (auto it = trajectory.rbegin() + 1; it != trajectory.rend(); ++it) {
            const auto [s_after, s_next] = *it;
            int a_next = find_best_action(tn, s_next);
            /*
            if (!(a_next >= 0 && a_next < 4)) {
                cout << trajectory.rend() - it << endl;
                env.showBoard();
                cout << a_next << endl;
                assert(a_next >= 0 && a_next < 4);
            }
            */
            board sim_env(s_next);
            // sim_env.showBoard();
            int r_next = sim_env.move(a_next);
            board_t s_after_next = sim_env.getState();
            float delta = r_next + tn.value(s_after_next) - tn.value(s_after);
            // cout << delta << " " << r_next << " " << tn.value(s_after_next) << " "
            //      << tn.value(s_after) << endl;
            tn.update(s_after, delta, alpha);
        }

        // cout << t + 1 << "score: " << score << endl;

        // logging
        if ((t + 1) % 1000 == 0) {
            float avg_score = 0;
            int max_score = 0;
            for (int i = 0; i < 1000; ++i) {
                avg_score += final_scores[i];
                max_score = max(max_score, final_scores[i]);
            }
            avg_score /= 1000;
            cout << "Episode: " << t + 1 << "\tScore: " << avg_score << "\tMax: " << max_score
                 << "\tMax Tile: " << max_tile << endl;
            final_scores.clear();
            max_tile = 0;
            if (avg_score > max_avg_score && avg_score > 61000) {
                max_avg_score = avg_score;
                tn.saveWeights(
                    "weights_best_" + to_string(t) + "_" +
                    to_string(static_cast<int>(max_avg_score)) + ".bin"
                );
            }

            /*
            if (t + 1 > 20000) {
                alpha *= 0.98;
            }
            */
        }
    }
    tn.saveWeights("weights_last.bin");
}