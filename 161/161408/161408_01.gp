\\ \r
\\ Nは使わない 

M=100;
a(n)= L=n^2+1; x='x+O('x^L); polcoef(1/prod(k=n+1, n^2, 1-x^k), n^2);

for(n=0, M, write("/Users/xxx/Desktop/b161408_1.txt", n, " ", a(n)))