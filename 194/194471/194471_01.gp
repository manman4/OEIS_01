\\ a(n) = n! * Sum_{k=0..n} (k+1)^(n-k-1) * binomial(2*k,k)/(n-k)!. 
a(n) = n! * sum(k=0, n, (k+1)^(n-k-1) * binomial(2*k,k)/(n-k)!);

for(n=0, 356, write("/Users/xxx/Desktop/b194471_1.txt", n, " ", a(n)))
