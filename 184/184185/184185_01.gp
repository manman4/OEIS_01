M=22;

a(n) = sum(k=n\2, n, (-1)^(n-k)*k!*binomial(k+1, n-k)); 
for(n=0, M, print1(a(n), ", "));