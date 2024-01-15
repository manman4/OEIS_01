M=20;

\\ a(n) = (1/(n+1)) * Sum_{k=0..n} 3^(n-k) * binomial(n+1,n-k) * binomial(2*k+2,k)
a(n) = (1/(n+1)) * sum(k=0, n, 3^(n-k) * binomial(n+1,n-k) * binomial(2*k+2,k) );
for(n=0, M, print1(a(n), ", "))
