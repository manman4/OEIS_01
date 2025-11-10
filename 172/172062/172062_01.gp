\\ a(n) = Sum_{k=0..n} (-2)^k * binomial(2*n+6,n-k).
a(n) = sum(k=0, n, (-2)^k * binomial(2*n+6, n-k));
for(n=0, 20, print1(a(n),", "))  

