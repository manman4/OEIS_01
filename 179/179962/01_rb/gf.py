# -*- coding: utf-8 -*-
"""各 k について G.f. の分子 P_k(x) と部分分数分解(閉形式)を計算する。"""
from fractions import Fraction as F

VALS = {
 2: [2]*10,
 3: [6,14,32,72,160,352,768,1664,3584,7680],
 4: [24,90,368,1496,6056,24440,98408,395576,1588136,6370040],
 5: [120,646,3984,25384,161136,1019616,6433728,40495488,254319360,1593945600],
 6: [720,5242,44304,399848,3661824,33461568,305193408,2779045440,25269799104,229495073088],
 7: [5040,47622,521606,6231544,76972416,957913824,11902581120,147689145984,1830235823616,22655103379968],
 8: [40320,479306,6564318,99133496,1572313392,25415753280,412583686272,6690112284672,108370981896192,1753844521231872],
}

def roots(k):
    r = [j*(k-j) for j in range(1, k//2 + 1)]
    if k % 2 == 1:
        r.append((k//2)*(k - k//2))   # 奇数 k は中央根が 2 重
    return r

def poly_mul(a, b):
    out = [0]*(len(a)+len(b)-1)
    for i, x in enumerate(a):
        for j, y in enumerate(b):
            out[i+j] += x*y
    return out

def denominator(k):
    """D(x) = prod (1 - lambda x)  (昇冪係数)"""
    d = [1]
    for lam in roots(k):
        d = poly_mul(d, [1, -lam])
    return d

def fmt_poly(coeffs, var='x'):
    terms = []
    for i, c in enumerate(coeffs):
        if c == 0: continue
        if i == 0: terms.append(f"{c}")
        else:
            co = "" if c == 1 else ("-" if c == -1 else f"{c}*")
            terms.append(f"{co}{var}" + (f"^{i}" if i > 1 else ""))
    s = " + ".join(terms).replace("+ -", "- ")
    return s or "0"

def fmt_denom(k):
    from collections import Counter
    c = Counter(roots(k))
    parts = []
    for lam in sorted(c):
        e = c[lam]
        parts.append(f"(1 - {lam}*x)" + (f"^{e}" if e > 1 else ""))
    return "".join(parts)

print("k | n0 | 分母 D(x) | 分子 P(x)")
print("--|----|-----------|----------")
for k in range(2, 9):
    d_poly = denominator(k)
    d = len(d_poly) - 1
    n0 = d + max(0, k-3)
    a = VALS[k]
    # P = (D*G) を x^(n0-1) まで切り詰め
    P = []
    for m in range(n0):
        s = 0
        for i, c in enumerate(d_poly):
            if m - i >= 0:
                s += c * a[m-i]
        P.append(s)
    while P and P[-1] == 0:
        P.pop()
    # 検算: n >= n0 で D*G の係数が 0 か
    ok = True
    for m in range(n0, len(a)):
        s = sum(d_poly[i]*a[m-i] for i in range(len(d_poly)) if m-i >= 0)
        if s != 0: ok = False
    print(f"{k} | {n0} | {fmt_denom(k)} | {fmt_poly(P)}   {'OK' if ok else 'NG***'}")

print()
print("閉形式 (n >= q = max(0,k-3) で有効):")
from fractions import Fraction as Fr
from collections import Counter
for k in range(2, 9):
    rs = roots(k)
    cnt = Counter(rs)
    d = len(rs)
    q = max(0, k-3)
    a = VALS[k]
    # 基底: 各根 lam の重複度 e に対し n^j lam^n (j=0..e-1)
    basis = []
    for lam in sorted(cnt):
        for j in range(cnt[lam]):
            basis.append((lam, j))
    # n = q..q+d-1 で連立
    M = [[Fr(n**j * lam**n) for (lam, j) in basis] for n in range(q, q+d)]
    rhs = [Fr(a[n]) for n in range(q, q+d)]
    # ガウス消去
    for col in range(d):
        piv = next(r for r in range(col, d) if M[r][col] != 0)
        M[col], M[piv] = M[piv], M[col]; rhs[col], rhs[piv] = rhs[piv], rhs[col]
        inv = M[col][col]
        M[col] = [x/inv for x in M[col]]; rhs[col] /= inv
        for r in range(d):
            if r != col and M[r][col] != 0:
                f = M[r][col]
                M[r] = [x - f*y for x, y in zip(M[r], M[col])]
                rhs[r] -= f*rhs[col]
    coef = rhs
    # 表示
    terms = {}
    for (lam, j), c in zip(basis, coef):
        terms.setdefault(lam, {})[j] = c
    parts = []
    for lam in sorted(terms):
        poly = terms[lam]
        ps = []
        for j in sorted(poly, reverse=True):
            c = poly[j]
            if c == 0: continue
            cs = str(c)
            ps.append(f"{cs}*n" if j == 1 else cs)
        inner = " + ".join(ps).replace("+ -", "- ")
        parts.append(f"({inner})*{lam}^n" if len(ps) > 1 or "n" in inner else f"{inner}*{lam}^n")
    expr = " + ".join(parts).replace("+ -", "- ")
    # 検算
    def ev(n):
        s = Fr(0)
        for (lam, j), c in zip(basis, coef):
            s += c * Fr(n**j * lam**n)
        return s
    ok = all(ev(n) == a[n] for n in range(q, len(a)))
    print(f"  k={k} (n>={q}): A_{k}(n) = {expr}   {'OK' if ok else 'NG***'}")
