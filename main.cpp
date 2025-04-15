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
    unsigned getFeature(const board_t board, const int pattern[6]) {
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

namespace TD_MCTS {
struct MCTS_Node {
    board_t state;
    int score;
    int visits;
    float total_reward;
    vector<MCTS_Node*> children;
    vector<int> children_actions;
    MCTS_Node* parent;
    bool untried_actions[4];

    MCTS_Node(board_t s, int score) : state(s), score(score), visits(0), total_reward(0) {
        board(s).getLegalActions(untried_actions);
    }
    bool fully_expanded() {
        for (int i = 0; i < 4; ++i) {
            if (untried_actions[i]) {
                return false;
            }
        }
        return true;
    }
    int get_untried_action() {
        for (int i = 0; i < 4; ++i) {
            if (untried_actions[i]) {
                return i;
            }
        }
        return -1;
    }
};

const int rollout_depth = 10;
const float c = 1.41;

namespace {
MCTS_Node* select_child(MCTS_Node* node) {
    MCTS_Node* best_child = nullptr;
    float best_value = -numeric_limits<float>::max();
    for (MCTS_Node* child : node->children) {
        float ucb_value =
            child->total_reward / child->visits + c * sqrt(log2(node->visits) / child->visits);
        if (ucb_value > best_value) {
            best_value = ucb_value;
            best_child = child;
        }
    }
    return best_child;
}

float rollout(board_t state, int score, TupleNetwork& tn) {
    board sim_env(state);
    int total_reward = score;
    bool done = false;
    for (int i = 0; i < rollout_depth && !done; ++i) {
        bool legal_actions[4];
        sim_env.getLegalActions(legal_actions);
        int action = -1;
        // TODO: random
        for (int j = 0; j < 4; ++j) {
            if (legal_actions[j]) {
                action = j;
                break;
            }
        }
        assert(action != -1);
        auto [next_state, reward, is_done, afterstate] = sim_env.step(action);
        total_reward += reward;
        done = is_done;
    }
    return total_reward + tn.value(sim_env.getState());
}

void backpropagate(MCTS_Node* node, float reward) {
    while (node != nullptr) {
        node->visits++;
        node->total_reward += reward;
        node = node->parent;
    }
}

int best_action(MCTS_Node* root) {
    int best_action = -1;
    float best_value = -numeric_limits<float>::max();
    for (int i = 0; i < root->children.size(); ++i) {
        auto& child = root->children[i];
        float ucb_value =
            child->total_reward / child->visits + c * sqrt(log2(root->visits) / child->visits);
        if (ucb_value > best_value) {
            best_value = ucb_value;
            best_action = root->children_actions[i];
        }
    }
    return best_action;
}
}  // namespace

void search(MCTS_Node* root, TupleNetwork& tn) {
    MCTS_Node* node = root;
    while (node->fully_expanded()) {
        MCTS_Node* child = select_child(node);
        if (child == nullptr) {
            break;
        }
        node = child;
    }

    board sim_env(node->state);
    if (!node->fully_expanded()) {
        int action = node->get_untried_action();
        assert(action != -1);
        auto [next_state, reward, is_done, afterstate] = sim_env.step(action);
        auto expanded_node = new MCTS_Node(next_state, node->score + reward);
        node->children.push_back(expanded_node);
        node->children_actions.push_back(action);
        expanded_node->parent = node;
        node = expanded_node;
    }

    int rollout_reward = rollout(node->state, node->score, tn);
    backpropagate(node, rollout_reward);
}
};  // namespace TD_MCTS

using namespace TD_MCTS;

int main() {
    GameSetting::init();
    TupleNetwork tn(patterns, groups, gn);
    tn.loadWeights("weights.bin");
    const int num_episodes = 500000;
    float alpha = 0.1;
    vector<int> final_scores;
    int max_tile = 0;
    for (int t = 0; t < num_episodes; ++t) {
        board env;
        bool done = false;
        int score = 0;
        vector<pair<board_t, board_t>> trajectory;
        while (!done) {
            // cout << "state " << hex << env.getState() << endl;
            board_t state = env.getState();
            int action;
            if (score < 20000) {
                action = find_best_action(tn, state);
            } else {
                auto root = new MCTS_Node(state, score);
                for (int i = 0; i < 500; ++i) {
                    search(root, tn);
                }
                action = best_action(root);
            }
            /*
            if (!(action >= 0 && action < 4)) {
                env.showBoard();
                cout << action << endl;
                assert(action >= 0 && action < 4);
            }
            */
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
            /*
            if (t + 1 > 20000) {
                alpha *= 0.98;
            }
            */
        }
    }
    tn.saveWeights("weights.bin");
}