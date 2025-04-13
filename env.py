import numpy as np
from board import Board
from math import log2


DOWN = 0
LEFT = 1
UP = 2
RIGHT = 3


class Game2048Env(Board):

    action_map = [UP, DOWN, LEFT, RIGHT]

    def __init__(self):
        super().__init__()
        self.score = 0

    @staticmethod
    def _to_array(s):
        a = np.zeros((4, 4), dtype=int)
        for i in range(16):
            v = s >> (i * 4) & 0xF
            if v != 0:
                a[i // 4][i % 4] = 2**v
        return a

    @staticmethod
    def _to_board_t(a):
        s = 0
        for i in reversed(range(16)):
            s <<= 4
            v = a[i // 4, i % 4]
            if v > 0:
                s |= int(log2(v))
        return s

    def get_state(self):
        s = super().get_state()
        return Game2048Env._to_array(s)

    def step(self, action):
        action = self.action_map[action]
        state, reward, done, afterstate = super().step(action)
        self.score += reward
        return (
            Game2048Env._to_array(state),
            self.score,
            done,
            Game2048Env._to_array(afterstate),
        )

    def is_game_over(self):
        return self.is_end()

    def reset(self, state=None, score=0):
        self.score = score
        if state is not None:
            if isinstance(state, np.ndarray):
                super().reset_with_state(Game2048Env._to_board_t(state))
            else:
                super().reset_with_state(state)
        else:
            super().reset()
        return self.get_state()

    def is_move_legal(self, action):
        return action in self.get_legal_actions()

    def render(self, action=None):
        pass

    @property
    def board(self):
        return self.get_state()

    @board.setter
    def board(self, state):
        if state is not None:
            if isinstance(state, np.ndarray):
                super().reset_with_state(Game2048Env._to_board_t(state))
            else:
                super().reset_with_state(state)


Game2048Env.init()

"""
b = Game2048Env()
b.reset()
print(b.get_state())

for m in range(4):
    state, reward, done, afterstate = b.step(m)
    print(m)
    print(afterstate)
    print(b.get_state())
    b.reset(afterstate)
    print(b.get_state())
"""
