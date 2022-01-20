default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(k=1, sqrtint(n), x^k^2/((1-x^k^2)*prod(j=1, k-1, 1-x^j)))};
M=10000;
v=v(M);
for(n=1, M, write("/Users/xxx/Desktop/b168656_1.txt", n, " ", polcoef(v, n)))