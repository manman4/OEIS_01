\\ For n>0,
\\ a(2*n)   =  (1/n) * Sum_{k=0..n-1} (-2)^k * binomial(2*n+k-1,k) * binomial(2*n-k-2,n-k-1),
\\ a(2*n+1) = -(2/n) * Sum_{k=0..n-1} (-2)^k * binomial(2*n+k+1,k) * binomial(2*n-k-2,n-k-1).
a(n) = my(m=n\2); if(n==1, 1, if(n%2==0, sum(k=0, m-1, (-2)^k * binomial(2*m+k-1,k) * binomial(2*m-k-2,m-k-1)) / m, -2 * sum(k=0, m-1, (-2)^k * binomial(2*m+k+1,k) * binomial(2*m-k-2,m-k-1)) / m));
for(n=1, 31, print1(a(n),", "));
\\ for(n=1, 1000, write("b143045_1.txt", n, " ", a(n)));