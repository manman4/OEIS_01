\\ a(n) = Sum_{k=0..n} 2^(n-k) * floor(k/5)
a(n) =sum(k=0,n, 2^(n-k)*(k\5));
for(n=1, 30, print1(a(n), ", ")); 

\\ a(n) = floor(2^(n+1)/31) - floor((n+1)/5)
a(n) = (2^(n+1)\31) - ((n+1)\5);
for(n=1, 30, print1(a(n), ", "));
