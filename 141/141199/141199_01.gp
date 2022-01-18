default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); 1/prod(k=1, n, 1-x^k/prod(j=1, k, 1-x^j))};
M=1000;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b141199_1.txt", n, " ", polcoef(v, n)))