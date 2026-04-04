\\ a(0) = 1; a(n) = a(n-1) - (n+2)/2 * Sum_{k=1..n-1} a(k) * a(n-k).
a(n) = if(n==0, 1, a(n-1) - (n+2)/2 * sum(k=1, n-1, a(k) * a(n-k)));
for(n=0, 12, print1(a(n),", "));