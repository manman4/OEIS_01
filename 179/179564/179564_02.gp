M=45;

\\ a(n) = Sum_{e=0..floor((n+3)/2)} binomial(n,2*e-3) * A271697(2*e-3,e).
E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
a(n) = sum(e=0, (n+3)\2, binomial(n, 2*e-3)*a271697(2*e-3, e));

for(n=1, M, print1(a(n),", "));