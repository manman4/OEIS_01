\\ A188775
isok(n) = sum(k=0, n, Mod(k, n)^k)==0;

\\ A341437
\\ isok(n) = sum(k=0, n, Mod(k, n)^(n-k))==0;
for(n=1, 10^4, if(isok(n), print1(n, ", ")))