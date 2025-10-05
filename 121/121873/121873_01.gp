M=1000;

\\ a(n) = (1/n) * Sum_{j=0..n-1} ( Sum_{i=0..n-j-1} binomial(i+n-1,n-1) * binomial(i+n,n-j-i-1) ) * binomial(n,j), n>0, a(0)=0.
a(n) =1/n * sum(j=0, n-1, sum(i=0, n-j-1, binomial(i+n-1,n-1)*binomial(i+n,n-j-i-1))*binomial(n,j));

for(n=1, M, write("/Users/xxx/Desktop/b121873_1.txt", n, " ", a(n)))

