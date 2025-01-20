M=18;

a(n) = n!*polcoef(1-exp(1-1/(1-4*x + x*O(x^n))^(1/4)), n);
for(n=1, 15, print1(a(n),", "))

\\ a(537) < 0.
\\ for(n=1, 550, if(a(n)<0, print1(n, ", ")))

bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
a000587(n) = bell(n, -1);
for(n=0, M, print1(a000587(n), ", "))

\\ a(n) = -Sum_{k=0..n} 4^(n-k) * |Stirling1(n,k)| * A000587(k).
a(n) = -sum(k=0, n, 4^(n-k) * abs(stirling(n, k, 1)) * a000587(k));
for(n=1, M, print1(a(n), ", "))
