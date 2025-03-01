M=16;

a(n, k=1) = if(k==0, 0^n, k*sum(j=0, n, binomial(5*n+k, j)/(5*n+k)*a(n-j, 5*j)));
for(n=0, M, print1(a(n),", "));

\\ Let a(n,k) = [x^n] A(x)^k.
\\ a(n,0) = 0^n; a(n,k) = k * Sum_{j=0..n} binomial(5*n+k,j)/(5*n+k) * a(n-j,5*j). 
b(n, k) = my(A=1); for(i=1, n, A = 1 + x * A^5 * subst(A, x, x*A^5)^5 + x*O(x^n) ); polcoef(A^k, n);
for(k=0, 10, for(n=0, M, print1(a(n, k)-b(n, k),", ")); print);

