M=20;

a107995(n) = sum(k=0, n, (2*n+2)^k*binomial(n+1+k, 2*k+1));
a323118(n) = sum(k=0, n, (2*n-2)^k*binomial(n+1+k, 2*k+1)); 
a(n) = (a323118(n)-a107995(n-2))/2;
for(n=2, M, print1(a(n), ", "));