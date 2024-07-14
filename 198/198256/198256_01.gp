\\ a(n) = Sum_{k=0..n+1} (k/(n+1))^2 * binomial(n+1,k)^4.
a(n) = sum(k=0, n+1, (k/(n+1))^2 * binomial(n+1,k)^4);
for(n=0, 18, print1(a(n), ", "))