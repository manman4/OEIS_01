M=37;

a365724(n) = sum(k=0, n\3, binomial(k, n-3*k)*binomial(n+1, k))/(n+1);
for(n=0, M, print1(a365724(n),", "))  

\\ a(n) = Sum_{k=0..n} binomial(n,k) * A365724(k).
a(n) = sum(k=0, n, binomial(n, k)*a365724(k));
for(n=0, M, print1(a(n),", "))

