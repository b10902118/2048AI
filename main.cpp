#include "pattern.h"
#include "bitboard.h"
#include "operation.h"
#include "feature.h"
#include "featureSet.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <array>
using namespace std;

const int WEIGHT_SIZE = 0xffffff;

class tupleNetwork {
   public:
    tupleNetwork(int patterns[][6], int group_sizes[], int gn) : pn(0), patterns(patterns) {
        for (int i = 0; i < gn; ++i) {
            pn += group_sizes[i];
        }
        weights = new float*[pn];
        int i = 0;
        for (int g = 0; g < gn; ++g) {
            float* weight = new float[WEIGHT_SIZE];
            for (int j = 0; j < group_sizes[g]; ++j) {
                weights[i] = weight;
                ++i;
            }
        }
    }

    ~tupleNetwork() {
        for (int i = 0; i < WEIGHT_SIZE; ++i) {
            delete[] weights[i];
        }
        delete[] weights;
    }

    float value(board_t board) {
        float value = 0;
        for (int i = 0; i < pn; ++i) {
            unsigned feature = getFeature(board, patterns[i]);
            value += weights[i][feature];
        }
        return value;
    }

    void update(board_t board, float delta, float alpha) {
        for (int i = 0; i < pn; ++i) {
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
    unsigned getFeature(board_t board, int pattern[6]) {
        unsigned feature = 0;
        for (int i = 0; i < 6; ++i) {
            feature <<= 4;
            feature |= (board >> pattern[i]) & 0xf;
        }
        return feature;
    }
    int pn;
    int (*patterns)[6];
    int* groups;
    float** weights;
};

int main() {}