\\ a(n) = Sum_{k=0..floor(n+2)/2)} 4^(n+2-2*k) * binomial(n+1-3*k/2,n+2-2*k).
a(n) = sum(k=0, (n+2)\2, 4^(n+2-2*k) * binomial(n+1-3*k/2,n+2-2*k));
for(n=0, 20, print1(a(n), ", "));   
