M=30;

a(n) = sum(k=0, n\2, k^(n-2*k)); 
for(n=0, M, print1(a(n), ", "));