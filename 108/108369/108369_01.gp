\\ a(n+1) = (-1)^n * Sum_{k=0..floor(n/3)} (-2)^k * binomial(n+2,3*k+2).
a(n) = (-1)^(n-1) * sum(k=0, (n-1)\3, (-2)^k * binomial(n-1+2,3*k+2));
for(n=0, 25, print1(a(n),", "))

  