a(n) = n!*polcoef(1-exp(1-1/(1-4*x + x*O(x^n))^(1/4)), n);
for(n=1, 15, print1(a(n),", "))

\\ a(537) < 0.
for(n=1, 550, if(a(n)<0, print1(n, ", ")))
