T(n, k) = sum(j=0, k, (n-k)^j*binomial(k+j, 2*j)*binomial(2*j, j)/(j+1));

for(n=0, 10, for(k=0, n, print1(T(n, k),", ")))  

\\ Ackermanの論文より
g(n, d) = if(n==0, 1, sum(k=0, n-1, binomial(n, k)*binomial(n, k+1)*(d-1)^k*d^(n-k))/n);
S(n, k) = g(k, n+1-k);

for(n=0, 10, for(k=0, n, print1(T(n, k)-S(n, k),", ")))
