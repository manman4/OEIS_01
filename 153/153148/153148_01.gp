\\ \r
\\ Nは使わない 
default(parisize, 120000000);

v(n)={q='q+O('q^(n+10)); sum(j=0, sqrt(n), q^(j^2)*prod(k=1, j, (1+q^(2*k-1))/(1+q^(2*k)))) };
M=10000;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b153148_gp_test.txt", n, " ", polcoef(v, n)))