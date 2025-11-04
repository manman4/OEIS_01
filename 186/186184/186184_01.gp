M=20;

a390386(n) = sum(k=0, n, binomial(n+6*k, k));
a390390(n) = sum(k=0, n, binomial(n+6*k+6, k));

\\ a(n) = A390386(n) - 7*A390390(n-1). 
a(n) = a390386(n) - 7*a390390(n-1);
for(n=0, M, print1(a(n),", "));
