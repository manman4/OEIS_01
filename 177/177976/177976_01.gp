M=12;

T(n, k) = sum(j=1, n, sumdiv(j, d, moebius(j/d)*binomial(d+k-2, d-1))); 
\\ upward
for(n=1, M, for(k=1, n, print1(T(n-k+1, k), ", ")));