M=26;

a(n) = if(n==0, 1, sum(k=1, n, isprime(k)*binomial(n-1, k-1)*a(n-k))); 
for(n=0, M, print1(a(n), ", "));