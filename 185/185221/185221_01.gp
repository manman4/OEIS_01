a(n) = n! * sum(k=0, n, stirling(n, k, 1)/(n-k+1)!);     
for(n=0, 30, print1(a(n),", "))         
