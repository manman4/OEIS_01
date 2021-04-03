\\ \r
\\ Nは使わない
\\ 念のため精度をあげる
default(realprecision, 100);

a(n) = my(m=(n*(n+1)/2)); if(issquare(m), 0, (sqrtint(m)+1)^2-m);
M=10000;
for(n=0, M, write("/Users/xxx/Desktop/b175032_2.txt", n, " ", a(n)))