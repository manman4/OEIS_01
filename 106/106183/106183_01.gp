\\ a(n) = Sum_{k=0..floor(n/2)} C(2*k,k) * C(2*(n-2*k),n-2*k).
a(n) = sum(k=0, n\2, binomial(2*k,k) * binomial(2*(n-2*k),n-2*k));
for(n=0, 20, print1(a(n),", "));

\\ a(n) = Sum_{k=0..floor(n/2)} 2^(n-k) * binomial(2*k,k) * binomial(n-k,k). 
b(n) = sum(k=0, n\2, 2^(n-k) * binomial(2*k,k) * binomial(n-k,k));
for(n=0, 100, print1(a(n)-b(n),", "));