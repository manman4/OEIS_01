M=18;

\\ a(n) = Sum_{k=0..n} binomial(n,k) * ( binomial(n,k) - binomial(n,k-1) )^3.
a(n) = sum(k=0, n, binomial(n,k) * (binomial(n,k) - binomial(n,k-1))^3);
for(n=0, M, print1(a(n), ", "))
