a(n) = (n-1)!*sum(k=0, n-1, n^k*binomial(n+k, n-1-k)/k!);
for(n=1, 20, print1(a(n),", "))                   

