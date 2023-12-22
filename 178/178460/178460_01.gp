\\ a(n) = Sum_{k=0..n} 2^(n-k) * floor(k/7).
a(n) = sum(k=0,n, 2^(n-k)*(k\7));
for(n=1, 30, print1(a(n), ", ")); 

\\ a(n) = floor(2^(n+1)/127) - floor((n+1)/7).
a(n) = (2^(n+1)\127) - ((n+1)\7);
for(n=1, 30, print1(a(n), ", "));
