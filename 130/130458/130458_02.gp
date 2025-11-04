M=20;

a389239(n) = sum(k=0, n, binomial(n+3*k, k));
a390383(n) = sum(k=0, n, binomial(n+3*k+3, k));

\\ a(n) = A389239(n) - 4*A390383(n-1). 
a(n) = a389239(n) - 4*a390383(n-1);
for(n=0, M, print1(a(n),", "));
