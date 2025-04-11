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
using namespace std;

const int WEIGHT_SIZE = 0xffffff + 1;

class tupleNetwork {
   public:
    tupleNetwork(const int patterns[][6], const int group_sizes[], int gn)
        : fn(0), patterns(patterns) {
        for (int i = 0; i < gn; ++i) {
            fn += group_sizes[i];
        }
        weights = new float*[fn];
        int i = 0;
        for (int g = 0; g < gn; ++g) {
            float* weight = new float[WEIGHT_SIZE];
            for (int j = 0; j < group_sizes[g]; ++j) {
                weights[i] = weight;
                ++i;
            }
        }
    }

    /*
    ~tupleNetwork() {
        for (int i = 0; i < WEIGHT_SIZE; ++i) {
            delete[] weights[i];
        }
        delete[] weights;
    }
    */

    float value(board_t board) {
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

    /*
    void saveWeights(const string& filename) {
            ofstream file(filename, ios::binary);
            if (!file) {
                    cerr << "Error opening file for writing: " << filename << endl;
                    return;
            }
            for (int i = 0; i < pn; ++i) {
                    file.write(reinterpret_cast<char*>(weights[i]), sizeof(float) * WEIGHT_SIZE);
            }
            file.close();
    }

    void loadWeights(const string& filename) {
            ifstream file(filename, ios::binary);
            if (!file) {
                    cerr << "Error opening file for reading: " << filename << endl;
                    return;
            }
            for (int i = 0; i < pn; ++i) {
                    file.read(reinterpret_cast<char*>(weights[i]), sizeof(float) * WEIGHT_SIZE);
            }
            file.close();
    }
    */

   private:
    unsigned getFeature(const board_t board, const int pattern[6]) {
        unsigned feature = 0;
        for (int i = 0; i < 6; ++i) {
            feature <<= 4;
            feature |= (board >> pattern[i]) & 0xf;
        }
        return feature;
    }
    int fn;
    const int (*patterns)[6];
    int* groups;
    float** weights;
};

int find_best_action(tupleNetwork& tn, const board_t state) {
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
            cout << action << ": " << reward << " " << value << endl;
            if (value > max_value) {
                max_value = value;
                best_action = action;
            }
        }
    }
    return best_action;
}

int main() {
    GameSetting::init();
    tupleNetwork tn(patterns, groups, gn);
    const int num_episodes = 1000;
    const float alpha = 0.1;
    vector<int> final_scores;
    /*
    for (int t = 0; t < num_episodes; ++t) {
        board env;
        bool done = false;
        int score = 0;
        vector<pair<board_t, board_t>> trajectory;
        while (!done) {
            // cout << "state " << hex << env.getState() << endl;
            int action = find_best_action(tn, env.getState());
            auto [next_state, reward, is_done, afterstate] = env.step(action);
            // cout << "action " << action << endl;
            // cout << "reward " << reward << endl;
            // cout << "afterstate " << hex << afterstate << endl;
            // cout << "next_state " << hex << next_state << endl;
            trajectory.push_back({afterstate, next_state});
            score += reward;
            done = is_done;
        }

        for (auto it = trajectory.rbegin() + 1; it != trajectory.rend(); ++it) {
            const auto [s_after, s_next] = *it;
            int a_next = find_best_action(tn, s_next);
            board sim_env(s_next);
            // sim_env.showBoard();
            int r_next = sim_env.move(a_next);
            board_t s_after_next = sim_env.getState();
            float delta = r_next + tn.value(s_after_next) - tn.value(s_after);
            tn.update(s_after, delta, alpha);
        }
        final_scores.push_back(score);
        if ((t + 1) % 100 == 0) {
            float avg_score = 0;
            for (int i = 0; i < 100; ++i) {
                avg_score += final_scores[t - i];
            }
            avg_score /= 100;
            cout << "Episode: " << t + 1 << ", Score: " << avg_score << endl;
        }
    }
    */
    // vector<pair<board_t, board_t>> trajectory;
    ifstream f("traj.txt");
    board_t s_after, s_next;
    int action, reward;
    while (f >> s_after >> s_next >> action >> reward) {
        cout << s_after << " " << s_next << " " << action << " " << reward << endl;
        int a_next = find_best_action(tn, s_next);
        board sim_env(s_next);
        sim_env.showBoard();
        // assert(a_next == action);
        //   sim_env.showBoard();
        int r_next = sim_env.move(a_next);
        cout << r_next << endl;
        assert(r_next == reward);
        board_t s_after_next = sim_env.getState();
        float delta = r_next + tn.value(s_after_next) - tn.value(s_after);
        tn.update(s_after, delta, alpha);
    }
    board env;
    bool done = false;
    int score = 0;
    while (!done) {
        // cout << "state " << hex << env.getState() << endl;
        int action = find_best_action(tn, env.getState());
        auto [next_state, reward, is_done, afterstate] = env.step(action);
        // cout << "action " << action << endl;
        // cout << "reward " << reward << endl;
        // cout << "afterstate " << hex << afterstate << endl;
        // cout << "next_state " << hex << next_state << endl;
        // cout << action << endl;
        // env.showBoard();
        score += reward;
        done = is_done;
    }
    cout << "Final score: " << score << endl;
}