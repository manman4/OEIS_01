M=20;

a(n) = (-1)^n*sum(k=0, n-1, (-n+1)^k*binomial(k, n-1-k));
for(n=1, M, print1(a(n), ", "));
