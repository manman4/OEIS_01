\\ a(n) = Sum_{k=0..n} 2^k * (-1)^(n-k) * binomial(n,k) * binomial(2*n+k,n).
a(n) = sum(k=0, n, 2^k * (-1)^(n-k) * binomial(n,k) * binomial(2*n+k,n));
for(n=0, 20, print1(a(n),", "));


   