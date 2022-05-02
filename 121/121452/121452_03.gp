M=30;

a(n) = n!*sum(k=0, n\2, abs(stirling(n-k, n-2*k, 1))/(n-k)!); 
for(n=0, M, print1(a(n), ", "));