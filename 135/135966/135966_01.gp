\\ T(n,k) = Sum_{j=0..k*(n-k)} binomial(j,k*(n-k)-j).
T(n,k) = sum(j=0, k*(n-k), binomial(j, k*(n-k)-j));
for(n=0, 10, for(k=0, n, print1(T(n,k),", "))); 