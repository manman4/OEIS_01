\\ a(n) = (1/(n+1)) * Sum_{k=0..floor(n/3)} (-1)^k * binomial(2*n+2,k) * binomial(3*n-3*k+1,2*n+1). 
a(n) = (1/(n+1)) * sum(k=0, n\3, (-1)^k * binomial(2*n+2,k) * binomial(3*n-3*k+1,2*n+1));
for(n=0, 30, print1(a(n), ", "));

