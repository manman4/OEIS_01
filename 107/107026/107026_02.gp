M=24;

\\ a(n) = (1/(4*n+1)) * Sum_{k=0..n} (4*k+1) * binomial(4*n+1,n-k).
a(n) = (1/(4*n+1)) * sum(k=0, n, (4*k+1) * binomial(4*n+1, n-k));
for(n=0, M, print1(a(n),", "));  

