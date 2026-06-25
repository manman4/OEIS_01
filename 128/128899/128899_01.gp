M=10;

C(n, k) = if(k>=0, binomial(n, k));
T(n, k) = C(2*n-2, n-k)-C(2*n-2, n-k-2);
for(n=0, M, for(k=0, n, print1(T(n, k), ", ")));


