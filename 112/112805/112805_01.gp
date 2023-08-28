\\ a(n+1) = Sum_{k=0..floor(n/4)} binomial(n-3*k-1,k) * binomial(n-2*k+1,n-4*k)/(n-2*k+1).

a(n) = sum(k=0, n\4, binomial(n-3*k-2,k) * binomial(n-2*k,n-4*k-1)/(n-2*k) );
for(n=1, 35, print1(a(n), ", "))


