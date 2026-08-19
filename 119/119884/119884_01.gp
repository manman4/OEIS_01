\\ a(0) = 1; a(n) = Sum_{k=0..n-1} binomial(n,k) * a(k) * b(n-k), where b(n) = n for odd n and b(n) = -1 for even n.
b(n) = if(n%2, n, -1);
a(n) = if(n==0, 1, sum(k=0, n-1, binomial(n,k) * a(k) * b(n-k)));
for(n=0, 20, print1(a(n),", "));