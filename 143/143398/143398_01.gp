M=10;

u(n, k) = if(k==0, 0, n\k);
T(n, k) = n!*sum(j=0, u(n, k), j^(n-k*j)/(k!^j*j!*(n-k*j)!));
for(n=0, M, for(k=0, n, print1(T(n,k),", "))) 