\\ a(n) = (n!)^2 * Sum_{k=0..n} (k/k!)^2.
a(n) = n!^2*sum(k=0, n, (k/k!)^2);
for(n=0, 20, print1(a(n), ", "));                               
