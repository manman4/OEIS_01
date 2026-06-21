\\ a(0) = 1; a(n) = (-1)^n * Sum_{i=1..floor(n/2)} (-1)^i/(2^(2*i-1)*(2*i)!) * Sum_{j=1..2*i} (-1)^j * j^(2*n) * binomial(4*i,2*i-j).
a(n) = if(n==0, 1, (-1)^n * sum(i=1, n\2, (-1)^i/(2^(2*i-1)*(2*i)!) * sum(j=1, 2*i, (-1)^j * j^(2*n) * binomial(4*i, 2*i-j))));
for(n=0, 15, print1(a(n), ", "));

M=298;
\\ for(n=0, M, write("b192060_1.txt", n, " ", a(n)));