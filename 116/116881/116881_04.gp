\\ a(n) = Sum_{k=0..n} 2^k * (-1)^(n-k) * (n-k+1) * binomial(2*n+1,k).
a(n) = sum(k=0, n, 2^k * (-1)^(n-k) * (n-k+1) * binomial(2*n+1, k));
for(n=0, 20, print1(a(n), ", "));



