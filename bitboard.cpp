// #include "stdafx.h"
#include "bitboard.h"

void board::reset() {
    m_board = 0;
    insertNewPiece();
    insertNewPiece();
}

void board::reset(board_t pos) { m_board = pos; }

int board::getNewPiece() {
    int r = configure::uRNG.rand() % 10;
    if (r == 0) return 2;
    else return 1;
};

int board::getEmptyPos() {
    int empty = 0;
    for (int i = 0; i < 64; i += 4) {
        if (((m_board >> i) & 0xf) == 0) empty++;
    }
    if (empty == 0) {
        return -1;
    }
    int iStart = configure::uRNG.rand() % empty;
    // cout << iStart << endl;
    bool existEmpty = false;

    for (int i = 0; i < 16; i++) {
        if (((m_board >> (4 * (15 - i))) & 0xf) == 0) {
            if (iStart == 0) {
                return i;
            }
            iStart--;
        }
    }

    return -1;
}

void board::insertNewPiece() {
    int pos = getEmptyPos();
    if (pos == -1) {
        cout << "insertNewPiece(): Wrong with getting new empty position" << endl;
        exit(1);
    } else {
        board_t num = getNewPiece();

        num = num << 4 * (15 - pos);
        board_t mask = 0x000f;
        mask = mask << 4 * (15 - pos);
        // cout << pos << endl;
        // cout << hex << num << endl;
        // cout << hex << mask << endl;
        mask = ~mask;
        // cout << hex << mask << endl;
        m_board = (m_board & mask) | num;
    }
};

bool board::isEnd() {
    for (int i = 0; i < 4; i++) {
        row_t r = operation::getRow(m_board, i);
        row_t c = operation::getCol(m_board, i);
        for (int j = 0; j < 3; j++) {
            if (((r >> 4 * j) & 0xf) == ((r >> 4 * (j + 1)) & 0xf) || ((r >> 4 * j) & 0xf) == 0) {
                return false;
            }
            if (((c >> 4 * j) & 0xf) == ((c >> 4 * (j + 1)) & 0xf) || ((c >> 4 * j) & 0xf) == 0) {
                return false;
            }
        }
        if (((r >> 12) & 0xf) == 0) return false;
        if (((c >> 12) & 0xf) == 0) return false;
    }

    return true;
}

bool board::isFull() {
    for (int i = 0; i < 16; i++) {
        if (((m_board >> 4 * i) & 0xf) == 0) return false;
    }

    return true;
};

row_t board::toRightRow(int dir, int i) {
    switch (dir) {
        case 0:
            return operation::reverseRow(operation::getCol(m_board, i));
        case 1:
            return operation::getRow(m_board, i);
        case 2:
            return operation::getCol(m_board, i);
        case 3:
            return operation::reverseRow(operation::getRow(m_board, i));
        default:
            cout << "toRightRow: Unknown direction" << endl;
            exit(1);
    }
}

void board::getLegalActions(bool a[4]) {
    for (int dir = 0; dir < 4; ++dir) {
        a[dir] = false;
        for (int i = 0; i < 4; i++) {
            if (configure::rightInfo[toRightRow(dir, i)].score != -1) {
                a[dir] = true;
                break;
            }
        }
    }
}

// 0 : up
// 1 : right
// 2 : down
// 3 : left
// must move
int board::move(int direct) {
    row_t row[4];
    int score = 0;
    bool notMove = true;
    switch (direct) {
        case 0:
            for (int i = 0; i < 4; i++) {
                row[i] = operation::reverseRow(operation::getCol(m_board, i));
                if (configure::rightInfo[row[i]].score == -1)
                    ;
                else {
                    row_t result = configure::rightInfo[row[i]].r;
                    score += configure::rightInfo[row[i]].score;
                    row[i] = result;
                    notMove = false;
                }
                // score += moveRight(row[i]);
                row[i] = operation::reverseRow(row[i]);
            }
            if (notMove) {
                cout << "Error: move(): Not moved" << endl;
                exit(1);
            } else {
                m_board = operation::setCols(row);
                return score;
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                row[i] = operation::getRow(m_board, i);
                if (configure::rightInfo[row[i]].score == -1)
                    ;
                else {
                    row_t result = configure::rightInfo[row[i]].r;
                    score += configure::rightInfo[row[i]].score;
                    row[i] = result;
                    notMove = false;
                }
                // score += moveRight(row[i]);
            }
            if (notMove) {
                cout << "Error: move(): Not moved" << endl;
                exit(1);
            } else {
                m_board = operation::setRows(row);
                return score;
            }
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                row[i] = operation::getCol(m_board, i);
                if (configure::rightInfo[row[i]].score == -1)
                    ;
                else {
                    row_t result = configure::rightInfo[row[i]].r;
                    score += configure::rightInfo[row[i]].score;
                    row[i] = result;
                    notMove = false;
                }
                // score += moveRight(row[i]);
            }
            if (notMove) {
                cout << "Error: move(): Not moved" << endl;
                exit(1);
            } else {
                m_board = operation::setCols(row);
                return score;
            }
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                row[i] = operation::reverseRow(operation::getRow(m_board, i));
                if (configure::rightInfo[row[i]].score == -1)
                    ;
                else {
                    row_t result = configure::rightInfo[row[i]].r;
                    score += configure::rightInfo[row[i]].score;
                    row[i] = result;
                    notMove = false;
                }
                row[i] = operation::reverseRow(row[i]);
            }
            if (notMove) {
                cout << "Error: move(): Not moved" << endl;
                exit(1);
            } else {
                m_board = operation::setRows(row);
                return score;
            }
            break;
        default:
            cout << "Wrong direction!" << endl;
            exit(1);
            break;
    }

    return score;
};

tuple<board_t, int, bool, board_t> board::step(int direct) {
    bool legal_actions[4];
    getLegalActions(legal_actions);
    int reward = 0;
    board_t afterstate = m_board;
    if (legal_actions[direct]) {
        reward = move(direct);
        afterstate = m_board;
    }
    // must moved => have space
    if (getEmptyPos() != -1) insertNewPiece();
    return make_tuple(m_board, reward, isEnd(), afterstate);
}

void board::showBoard() {
    for (int i = 0; i < 4; i++) {
        row_t r = operation::getRow(m_board, i);
        for (int j = 3; j >= 0; j--) {
            cout << configure::tile_score[((r >> 4 * j) & 0xf)] << "\t";
        }
        cout << endl;
    }
    cout << endl;
}

namespace py = pybind11;
PYBIND11_MODULE(board, m) {
    py::class_<board>(m, "Board")
        .def(py::init<>())
        .def(py::pickle(
            [](const board& b) { // dump
                return b.getState();
            },
            [](board_t s) { // load
                return board(s);
            }
        ))
        .def("init", &GameSetting::init)
        .def("reset", (void (board::*)())&board::reset)
        .def("reset_with_state", (void (board::*)(board_t))&board::reset)
        .def("move", &board::move)
        .def("step", &board::step)
        .def("get_state", &board::getState)
        .def("is_end", &board::isEnd)
        .def("show", &board::showBoard)
        .def(
            "get_legal_actions",
            [](board& b)
            {
                bool actions[4];
                b.getLegalActions(actions);
                vector<int> ret;
                for (int i = 0; i < 4; ++i) {
                    if (actions[i]) ret.push_back(i);
                }
                return ret;
            }
        );
}