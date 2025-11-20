M=15;

\\ a(0) = 1; a(n) = Sum_{k=1..n} (2*(k-1)! + 1) * binomial(n-1,k-1) * a(n-k).
a(n) = if(n==0, 1, sum(k=1, n, (2*(k-1)! + 1) * binomial(n-1, k-1) * a(n-k)));
for(k=0, M, print1(a(k), ", "));