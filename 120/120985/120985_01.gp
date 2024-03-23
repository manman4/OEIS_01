\\ a(n) = (1/(n+1)) * Sum_{k=0..floor(n/2)} (-3)^k * binomial(n+1,k) * binomial(3*n-3*k+3,n-2*k). 
a(n) = sum(k=0, n\2, (-3)^k * binomial(n+1, k) * binomial(3*n-3*k+3, n-2*k)) / (n+1);
for(n=0, 25, print1(a(n),", "))  

\\ a(n) = (1/(n+1)) * Sum_{j..floor(n/3)} 3^(n-3*j) * binomial(n+1,2*j+1) * binomial(n-2*j,j)
b(n) = sum(j=0, n\3, 3^(n-3*j) * binomial(n+1, 2*j+1) * binomial(n-2*j, j)) / (n+1);
for(n=0, 25, print1(a(n)-b(n),", "))

