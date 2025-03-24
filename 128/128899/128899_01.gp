M=10;

T(n, k) = binomial(2*n-2, n-k)-binomial(2*n-2, n-k-2);
for(n=0, M, for(k=0, n, print1(T(n, k), ", ")));


