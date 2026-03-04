\\ a(0) = 1; a(n) = (1/n) * Sum_{k=0..floor((n-1)/2)} 3^(n-1-2*k) * binomial(2*n-2-k,n-1) * binomial(n-1-k,k).
a(n) = if(n==0, 1, (1/n) * sum(k=0, (n-1)\2, 3^(n-1-2*k) * binomial(2*n-2-k,n-1) * binomial(n-1-k,k)));
for(n=0, 20, print1(a(n),", "));