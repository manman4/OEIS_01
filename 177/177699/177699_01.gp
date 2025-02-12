a(n) = (-1)^n*sum(k=1, n\2, (n-2*k)!*binomial(n, 2*k-1)); 
for(n=0, 30, print1(a(n), ", "))
