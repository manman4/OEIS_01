# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A116469(n, k):
    if n == 1 or k == 1: return 1
    universe = tl.grid(n-1, k-1)
    GraphSet.set_universe(universe)
    spanning_trees = GraphSet.trees(is_spanning=True)
    return spanning_trees.len()

cnt = 1
for i in range(11):
    for j in range(i + 1):
        print(str(cnt) + ' ' + str(A116469(j + 1, i - j + 1)))
        cnt += 1