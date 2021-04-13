\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sqrt((1-8*x+sqrt(1+64*x^2))/2)};
M=1000;
v=v(M);
for(n=0, M, i=polcoef(v, n); if((i<0)+#digits(i) > 1000, break); write("C:\\Users\\xxx\\Desktop\\b193619_1.txt", n, " ", i))
