\\ a(n) = (1/(n+1)) * Sum_{k=0..floor(n/3)} (n-3*k+1) * binomial(n+1,k).
a(n) = (1/(n+1)) * sum(k=0, n\3, (n-3*k+1) * binomial(n+1,k));
for(n=0, 25, print1(a(n),", "))   
      