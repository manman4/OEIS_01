# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A145416(n):
    universe = tl.grid(2 * n - 1, 6)
    GraphSet.set_universe(universe)
    cycles = GraphSet.cycles(is_hamilton=True)
    return cycles.len()

cnt = 1
for i in range(1, 501):
    print(str(cnt) + ' ' + str(A145416(i)))
    cnt += 1

