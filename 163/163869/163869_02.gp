M=22;

\\ a(n) = Sum_{k=0..n} (2*k+1) * binomial(2*k,k) * binomial(n,n-k).
a(n) = sum(k=0, n, (2*k+1) * binomial(2*k,k) * binomial(n,n-k));
for(n=0, M, print1(a(n), ", "));