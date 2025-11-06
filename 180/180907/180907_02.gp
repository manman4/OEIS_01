\\ a(n) = 4^(n-1) - Sum_{k=0..n-2} 4^(n-2-k) * A036765(k).
a036765(n) = sum(j=0, n\2, binomial(n+1, n-2*j)*binomial(n+1, j))/(n+1);
a(n) = 4^(n-1) - sum(k=0, n-2, 4^(n-2-k) * a036765(k));
for(n=1, 26, print1(a(n),", "));