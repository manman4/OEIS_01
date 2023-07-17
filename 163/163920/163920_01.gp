M=21;

a(n) = n/4*sumdiv(n, d, (-1)^(n+d)*(d+1)*(n/d+1));
for(n=1, M, print1(a(n), ", "))