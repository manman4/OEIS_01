\\ a(n) = [x^n] 1/((1-x) * (1-2*x)^(n+1)).
a(n) = polcoef( 1/((1-x) * (1-2*x)^(n+1) + x*O(x^n)), n);
for(n=0, 25, print1(a(n), ", "));


