M=20;

\\ a(n) = Sum_{k=0..n} k^2 * (-1)^k * 3^(n-k) * binomial(n,k).
a(n) = sum(k=0, n, k^2 * (-1)^k * 3^(n-k) * binomial(n,k));
for(n=0, M, print1(a(n),", "))  
