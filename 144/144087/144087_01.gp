\\ a(n) = (n!/2) * Sum_{k=0..n-2} binomial(k,n-2-k)/(n-2-k)!. 
a(n) = n!/2 * sum(k=0, n-2, binomial(k,n-2-k)/(n-2-k)!);
for(n=0, 20, print1(a(n), ", "))


