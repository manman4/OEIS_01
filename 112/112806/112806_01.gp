a(n) = sum(k=0, n\2, binomial(n-k-1,k) * binomial(2*n-k+1,n-2*k)/(2*n-k+1) );
for(n=0, 23, print1(a(n), ", "))


