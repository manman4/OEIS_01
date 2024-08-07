\\ a(n) = n! * Sum_{k=0..n} binomial(k,n-k)/(n-k)!.
a(n) = n! * sum(k=0, n, binomial(k,n-k)/(n-k)!);
for(n=0, 20, print1(a(n), ", "))


