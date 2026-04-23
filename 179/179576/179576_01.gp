E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
a062866(n, k) = sum(e=0, (n+k)\2, binomial(n, 2*e-k)*a271697(2*e-k, e));

\\ a(n) = A062866(n+9,n).
for(n=1, 25, print1(a062866(n+9, n),", "));
