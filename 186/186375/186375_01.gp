\\ a(n) = Sum_{k=0..n} binomial(n+k,k) * binomial(2*k,k) * binomial(2*(n-k),n-k).
a(n) = sum(k=0, n, binomial(n+k, k) * binomial(2*k, k) * binomial(2*(n-k), n-k));
for(n=0, 25, print1(a(n), ", "));