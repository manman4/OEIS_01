T(n, k) = sum(j=k, n\2, (-1)^(j-k)*(j+1)*binomial(j, k)*binomial(2*n-3*j+1, n-2*j)/(2*n-3*j+1));
for(n=0, 13, for(k=0, n\2, print1(T(n, k), ", ")));