a(n) = (1/(n+1)) * sum(k=0, n\2, binomial(2*n+2,k) * binomial(n-k-1,n-2*k) );
for(n=0, 30, print1(a(n),", "))       