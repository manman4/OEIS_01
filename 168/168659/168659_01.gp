default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(i=1, n, sum(j=1, (n+1)\(i+1), x^((i+1)*j-1)*prod(k=1, j-1, (1-x^(i*j+k-1))/(1-x^k))))};
M=1000;
v=v(M);
for(n=1, M, write("/Users/xxx/Desktop/b168659_1.txt", n, " ", polcoef(v, n))) 