\\ a(0) = a(1) = 1; a(n) = a(n-1) - 3*a(n-2) + Sum_{k=0..n-2} a(k) * a(n-2-k).
a(n) = if(n<2, 1, a(n-1) - 3*a(n-2) + sum(k=0, n-2, a(k) * a(n-2-k)));
for(n=0, 18, print1(a(n),", "));

