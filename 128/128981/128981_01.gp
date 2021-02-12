\\ A128981
isok(n) = sum(k=1, n, Mod(k, n)^k)==0;

\\ A341436
\\ isok(n) = sum(k=1, n, Mod(k, n)^(n+1-k))==0;
for(n=1, 10^4, if(isok(n), print1(n, ", ")))