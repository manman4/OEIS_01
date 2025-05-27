\\ G.f.: exp(2 * Sum_{k>=1} (-1)^(k-1) * A000670(k) * x^k/k).
a000670(n) = sum(k=0, n, k! * stirling(n, k, 2));
for(n=0, 20, print1(a000670(n), ", "));
my(N=20, x='x+O('x^N)); Vec(exp(2 * sum(k=1, N, (-1)^(k-1) * a000670(k) * x^k/k)))
