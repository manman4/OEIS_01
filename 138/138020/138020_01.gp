\\ a(n) = (2^n/(n+1)) * Sum_{k=0..n} binomial(n/2+k-1/2,k) * binomial(n/2+1/2,n-k).
a(n) = (2^n/(n+1)) * sum(k=0, n, binomial(n/2+k-1/2,k) * binomial(n/2+1/2,n-k));
for(n=0, 25, print1(a(n),", "))
