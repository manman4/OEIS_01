M=22;

a(n) = 2*sum(k=0, (n+1)\2, (4^k-1)*abs(stirling(n+1, 2*k, 1))*bernfrac(2*k));
for(n=1, M, print1(a(n), ", "));