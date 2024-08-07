\\ a(n) = n! * Sum_{k=0..n-1} binomial(k,n-1-k)/(n-1-k)!. 
a(n) = n! * sum(k=0, n-1, binomial(k,n-1-k)/(n-1-k)!);
for(n=0, 20, print1(a(n), ", "))


