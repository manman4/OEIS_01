a(n) = round((-sqrt(n-1)*I)^(n-1)*polchebyshev(n-1, 2, sqrt(n-1)*I/2));

M=20;
for(n=1, M, print1(a(n), ", ")); 