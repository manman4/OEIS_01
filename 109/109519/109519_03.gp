M=20;

a(n) = round(-sqrt(n-1)^(n-1)*polchebyshev(n-1, 2, sqrt(n-1)/2)); 
for(n=1, M, print1(a(n), ", "));