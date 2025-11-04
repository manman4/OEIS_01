M=20;

a390384(n) = sum(k=0, n, binomial(n+4*k, k));
a390388(n) = sum(k=0, n, binomial(n+4*k+4, k));

\\ a(n) = A390384(n) - 5*A390388(n-1).
a(n) = a390384(n) - 5*a390388(n-1);
for(n=0, M, print1(a(n),", "));
