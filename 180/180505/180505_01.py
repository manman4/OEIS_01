# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A321172(n, k):
    universe = tl.grid(n - 1, k - 1)
    GraphSet.set_universe(universe)
    cycles = GraphSet.cycles(is_hamilton=True)
    return cycles.len()

for i in range(1, 51):
    print(str(i) + ' ' + str(A321172(2 * i, 11)))
