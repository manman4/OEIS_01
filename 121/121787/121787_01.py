# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A121787(n):
    universe = tl.grid(2 * n, 2 * n)
    GraphSet.set_universe(universe)
    start, goal = 1, (2 * n + 1) * (2 * n + 1)
    paths = GraphSet.paths(start, goal).including((start + goal) // 2)
    return paths.len()

print([A121787(n) for n in range(1, 7)]) 