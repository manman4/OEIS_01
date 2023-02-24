M=11;

a(n) = sum(k=0, n, binomial(n, k)*binomial(2^k+n-1, n));
for(n=0, M, print1(a(n), ", "));