M=45;

E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
a(n) = sum(e=0, (n+1)\2, binomial(n, 2*e-1)*a271697(2*e-1, e));
cnt=0;
for(n=1, M, write("b179562_1.txt", cnt, " ", a(n)); cnt++);