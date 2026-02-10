\\ a(1) = 1; a(n) = (-1)^n * Sum_{k=1..n-1} a(k)*a(n-k).
a(n) = if(n==1, 1, (-1)^n * sum(k=1, n-1, a(k)*a(n-k)));
for(n=1, 16, print1(a(n), ", "));
