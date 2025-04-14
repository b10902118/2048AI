import numpy as np


def generate_symmetries(pattern):
    # TODO: Generate 8 symmetrical transformations of the given pattern.
    ordered_pattern = np.full_like(pattern, -1, dtype=np.int32)
    cnt = 0
    for i in range(ordered_pattern.shape[0]):
        for j in range(ordered_pattern.shape[1]):
            if pattern[i, j]:
                ordered_pattern[i, j] = cnt
                cnt += 1

    ordered_syms = [
        ordered_pattern,
        np.rot90(ordered_pattern, 1),
        np.rot90(ordered_pattern, 2),
        np.rot90(ordered_pattern, 3),
    ]
    ordered_syms += [np.fliplr(s) for s in ordered_syms]

    coord_syms = []
    for order in ordered_syms:
        coords = [None] * cnt
        for i in range(order.shape[0]):
            for j in range(order.shape[1]):
                if order[i, j] != -1:
                    coords[order[i, j]] = 4 * i + j  # (i, j)
        coord_syms.append(coords)
    return coord_syms


# fmt: off
patterns = [
    [[1,1,1,0],
     [1,1,1,0],
     [0,0,0,0],
     [0,0,0,0]],

    [[0,1,1,0],
     [0,1,1,0],
     [0,1,0,0],
     [0,1,0,0]],

    [[1,1,1,1],
     [1,1,0,0],
     [0,0,0,0],
     [0,0,0,0]],

    [[1,1,0,0],
     [0,1,1,1],
     [0,0,1,0],
     [0,0,0,0]],

    [[1,1,1,0],
     [0,1,0,0],
     [0,1,1,0],
     [0,0,0,0]],

    [[1,1,0,0],
     [0,1,0,0],
     [0,1,0,0],
     [0,1,1,0]],

    [[1,1,0,0],
     [0,1,0,0],
     [1,1,0,0],
     [0,1,0,0]],

    [[1,1,1,0],
     [1,0,1,0],
     [0,0,1,0],
     [0,0,0,0]],
]
# fmt: on
patterns = [np.array(p, dtype=np.int32) for p in patterns]

ordered_patterns = []
for p in patterns:
    ordered_patterns += generate_symmetries(p)

print("{")
for op in ordered_patterns:
    print("    {", end="")
    print(", ".join([str(x) for x in op]), end="")
    print("},")
print("}")

print(len(ordered_patterns))
print(",".join(["8"] * (len(ordered_patterns) // 8)))
