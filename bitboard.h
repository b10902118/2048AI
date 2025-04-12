#ifndef _BITBOARD_H
#define _BITBOARD_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "rng.h"
#include "operation.h"
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <tuple>
#include <cstdlib>

using namespace std;

class board {
   public:
    board() { reset(); };

    board(board_t pos) { m_board = pos; };

    // initialize the board with two random piece
    void reset();

    // initialize the board with given starting condition
    void reset(board_t b);

    // return >= 0 value (eaten score retrieve) if it is a move, and move
    // else return -1
    int move(int direct);

    tuple<board_t, int, bool, board_t> step(int direct);

    board_t getState() const { return m_board; };

    void getLegalActions(bool a[4]);

    // return true if cannot move anymore
    bool isEnd();

    void showBoard();

   private:
    row_t toRightRow(int dir, int n);

    // 1 for piece num 2
    // 2 for piece num 4
    int getNewPiece();

    // return a new vacant position
    // if no empty pos -> return invalidPos
    int getEmptyPos();

    //
    void insertNewPiece();

    // return true if it is full
    bool isFull();

    // board status
    board_t m_board;
};

#endif