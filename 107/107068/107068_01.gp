\\ a(n) = Sum_{k=0..n+4} (-1)^(n-k)*C(n+4, k) * Sum_{j=0..floor(k/4)} C(k-3*j, j).
a(n) = sum(k=0, n+4, (-1)^(n-k)*binomial(n+4, k) * sum(j=0, k\4, binomial(k-3*j, j)));
for(n=0, 30, print1(a(n), ", "));

\\ a(n) = (-1)^n * Sum_{k=0..floor(n/4)} binomial(n-k+2,n-4*k).
b(n) = (-1)^n * sum(k=0, n\4, binomial(n - k + 2, n - 4*k));
for(n=0, 100, print1(a(n)-b(n), ", "));