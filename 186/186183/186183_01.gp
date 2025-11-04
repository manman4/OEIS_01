M=20;

a390385(n) = sum(k=0, n, binomial(n+5*k, k));
a390389(n) = sum(k=0, n, binomial(n+5*k+5, k));

\\ a(n) = A390385(n) - 6*A390389(n-1). 
a(n) = a390385(n) - 6*a390389(n-1);
for(n=0, M, print1(a(n),", "));
