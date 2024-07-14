\\ a(n) = Sum_{k=0..n} (k/n)^3 * binomial(n,k)^4. 
a(n) = sum(k=0, n, (k/n)^3 * binomial(n, k)^4);
for(n=1, 50, print1(a(n), ", "))