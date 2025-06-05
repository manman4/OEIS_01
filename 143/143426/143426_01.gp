M=20;

\\ a(n,0) = 0^n; a(n,k) = k * Sum_{j=0..n} binomial(n-j+k,j)/(n-j+k) * a(n-j,2*j).
a(n, k) = if(k==0, 0^n, k*sum(j=0, n, binomial(n-j+k, j)/(n-j+k)*a(n-j, 2*j)));
for(n=0, M, print1(a(n, 1),", "));

