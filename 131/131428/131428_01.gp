M=18;

\\ a(n) = Sum_{k=0..n} A080233(n,k)^2 = Sum_{k=0..n} A156644(n,k)^2.
\\ a(n) = Sum_{k=0..n} ( binomial(n,k) - binomial(n,k-1) )^2
a(n) = sum(k=0, n, (binomial(n,k) - binomial(n,k-1))^2);
for(n=0, M, print1(a(n), ", "))
