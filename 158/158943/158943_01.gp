\\ a(n) = Sum_{k=0..floor(n/2)} binomial(2*n-3*k-1,k).
a(n) = sum(k=0, n\2, binomial(2*n-3*k-1, k));
for(n=1, 40, print1(a(n),", "))
