a(n) = sum(k=0, n\2, binomial(n+3*k,k) * binomial(n+2*k,n-2*k) / (4*k+1) );
for(n=0, 30, print1(a(n),", "))  

