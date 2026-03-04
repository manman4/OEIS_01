\\ a(0) = 1; a(n) = (n-1)! * Sum_{k=0..floor((n-1)/2)} n^k * (2*n)^(n-1-2*k) * binomial(n-1-k,k)/(n-1-k)!. 
a(n) = if(n==0, 1, (n-1)! * sum(k=0, (n-1)\2, n^k * (2*n)^(n-1-2*k) * binomial(n-1-k,k)/(n-1-k)!));
for(n=0, 19, print1(a(n),", "));