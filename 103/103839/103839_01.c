#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// A103839: number of permutations of 1..n where adjacent sums are prime.
// Safe + fast: DP over subsets (Held–Karp), O(n^2 2^n).

static bool is_prime(int x) {
    if (x < 2) return false;
    if (x % 2 == 0) return x == 2;
    for (int d = 3; d * d <= x; d += 2) {
        if (x % d == 0) return false;
    }
    return true;
}

int main(void) {
    int Nmax;
    printf("Enter Nmax: ");
    if (scanf("%d", &Nmax) != 1 || Nmax < 1 || Nmax > 25) {
        fprintf(stderr, "Nmax must be in 1..25\n");
        return 1;
    }

    FILE *out = fopen("b103839.txt", "w");
    if (!out) {
        perror("b103839.txt");
        return 1;
    }
    setvbuf(out, NULL, _IOLBF, 0); // line-buffered for sequential output

    for (int n = 1; n <= Nmax; n++) {
        // Build adjacency (1..n)
        bool adj[25][25] = {{0}};
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                if (is_prime((i + 1) + (j + 1))) adj[i][j] = true;
            }
        }

        int size = 1 << n;
        // Use 128-bit to avoid overflow in intermediate counts.
        unsigned __int128 *dp = (unsigned __int128 *)calloc(
            (size_t)size * (size_t)n, sizeof(unsigned __int128));
        if (!dp) {
            fprintf(stderr, "out of memory\n");
            return 1;
        }

        // Initialize: paths of length 1
        for (int i = 0; i < n; i++) {
            dp[(1 << i) * n + i] = 1;
        }

        for (int mask = 1; mask < size; mask++) {
            for (int last = 0; last < n; last++) {
                uint64_t cur = dp[mask * n + last];
                if (cur == 0) continue;
                for (int nxt = 0; nxt < n; nxt++) {
                    if (mask & (1 << nxt)) continue;
                    if (!adj[last][nxt]) continue;
                    int nmask = mask | (1 << nxt);
                    dp[nmask * n + nxt] += cur;
                }
            }
        }

        unsigned __int128 total = 0;
        int full = size - 1;
        for (int last = 0; last < n; last++) {
            total += dp[full * n + last];
        }

        // Totals fit in 64-bit for n<=24 in OEIS, but keep safe conversion.
        unsigned long long outv = (unsigned long long)total;
        fprintf(out, "%d %llu\n", n, outv);
        fflush(out);
        free(dp);
    }

    fclose(out);
    return 0;
}
