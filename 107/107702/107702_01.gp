T(n, k) = sum(j=0, k, (n-k)^j*binomial(k+j, 2*j)*binomial(2*j, j)/(j+1));

for(n=0, 10, for(k=0, n, print1(T(n, k),", ")))  