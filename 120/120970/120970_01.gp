M=18;

\\ b(n,0) = 0^n; b(n,k) = k * Sum_{j=0..n} binomial(2*n+k,j)/(2*n+k) * b(n-j,2*j).
\\ a(n) = b(n-1,2) for n > 0.
b(n, k) = if(k==0, 0^n, k*sum(j=0, n, binomial(2*n+k, j)/(2*n+k)*b(n-j, 2*j)));
a(n) = if(n==0, 1, b(n-1, 2)); 
for(n=0, M, print1(a(n),", "));

\\ Let b(n,k) = [x^n] B(x)^k, where B(x) is the g.f. of A120971.
\\ b(n,0) = 0^n; b(n,k) = k * Sum_{j=0..n} binomial(2*n+k,j)/(2*n+k) * b(n-j,2*j).
a(n, k) = my(A=1); for(i=1, n, A = 1 + x * A^2 * subst(A, x, x*A^2)^2 + x*O(x^n) ); polcoef(A^k, n);
for(k=0, 10, for(n=0, M, print1(a(n, k)-b(n, k),", ")); print);

