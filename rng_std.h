#ifndef _RNG_H
#define _RNG_H

#include <random>
#include <ctime>
#include <iostream>

using namespace std;

class uniform_RNG {
   public:
    uniform_RNG() : rng(std::random_device{}()) {
        // Seed the random number generator with the current time
        srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    ~uniform_RNG() = default;

    void srand(unsigned int seed) {
        if (seed == 0) {
            rng.seed(std::random_device{}());
        } else {
            rng.seed(seed);
        }
    }

    unsigned int rand() {
        // Generate a random number in the full range of unsigned int
        return dist(rng);
    }

   private:
    std::mt19937 rng;                                  // Mersenne Twister random number generator
    std::uniform_int_distribution<unsigned int> dist;  // Uniform distribution for unsigned int
};

#endif