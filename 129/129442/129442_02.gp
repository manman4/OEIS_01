\\ a(n) = (1/(n+1)) * Sum_{k=0..floor(n/2)} (-1)^k * binomial(n+k,k) * binomial(3*n-k+1,n-2*k). 
a(n) = sum(k=0, n\2, (-1)^k*binomial(n+k, k)*binomial(3*n-k+1, n-2*k))/(n+1);
for(n=0, 30, print1(a(n), ", "));  