a(n) = sum(k=0, n\2, (-1)^k * binomial(3*n-5*k,k) * binomial(3*n-6*k,n-2*k) / (2*n-4*k+1) ); 
for(n=0, 20, print1(a(n),", ")) 