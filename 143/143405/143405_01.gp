M=30;

a(n) = sum(k=0, n, k!*sum(j=0, k\2, 1/(j!*(k-2*j)!))*stirling(n, k, 2)); 
for(n=0, M, print1(a(n), ", "));