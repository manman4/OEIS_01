T(n, k) = abs(polresultant(polchebyshev(n, 2, x/2), polchebyshev(k, 2, I*x/2))); 
for(n=1, 10, for(k=1, n, print1(T(k, n-k+1),", ")));