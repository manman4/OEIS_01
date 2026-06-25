\\ a(n) = (1/n) * Sum_{k=0..n} (-1)^(n-k-1) * binomial(n,k) * binomial(2*n-3*k,n-k-1) for n > 0. 
C(n, k) = if(k>=0, binomial(n, k));
a(n) = if(n==0, -1, sum(k=0, n, (-1)^(n-k-1)*C(n, k)*C(2*n-3*k, n-k-1))/n);
for(n=0, 20, print1(a(n), ", "))
