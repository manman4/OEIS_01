\\ a(0) = 1; a(n) = 3^(n-1) * Sum_{k=0..n-1} a(k) * a(n-1-k).
a(n) = if(n==0, 1, 3^(n-1) * sum(k=0, n-1, a(k) * a(n-1-k)));
for(n=0, 10, print1(a(n),", "));

