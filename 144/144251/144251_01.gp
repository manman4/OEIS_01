M=20;

a(n) = if(n==0, 1, sum(k=0, n-1, binomial(2*n-2-k, k)*a(k)));
for(n=0, M, print1(a(n), ", "));