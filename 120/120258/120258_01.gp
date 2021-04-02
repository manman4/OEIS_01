M=9;

T(n, k) = prod(j=0, k-1, binomial(2*n-2*k+j, n-k)/binomial(n-k+j, j));
for(n=0, M, for(k=0, n, print1(T(n, k), ", ")))