\\ a(n) = Sum_{k=0..floor(2*n/5)} binomial(2*n-4*k,k). 
a(n) = sum(k=0, 2*n\5, binomial(2*n-4*k, k));
for(n=0, 100, print1(a(n), ", "))