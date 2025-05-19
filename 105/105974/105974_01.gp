M=11;

\\ A(n,k) = [x^n] Product_{j=0..n} (1 + (j+k)*x).
a(n, k) = polcoef(prod(j=0, n, 1+(j+k)*x), n);
for(n=0, M, for(k=0, n, print1(a(k, n-k), ", ")));

\\ Main diagonal gives A384024.
for(n=0, M, print1(a(n, n), ", "));