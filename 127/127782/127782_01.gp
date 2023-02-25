M=11;

a(n) = if(n==0, 1, sum(k=0, (n-1)\2, binomial(n-1-k, k)*a(n-1-k)));
for(n=0, M, print1(a(n), ", "));