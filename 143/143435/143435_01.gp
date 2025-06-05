M=20;

\\ a(n,0) = 0^n; a(n,k) = k * Sum_{j=0..n} binomial(n-j+k,j)/(n-j+k) * a(n-j,3*j).
a(n, k) = if(k==0, 0^n, k*sum(j=0, n, binomial(n-j+k, j)/(n-j+k)*a(n-j, 3*j)));
for(n=0, M, print1(a(n, 1),", "));

\\ G.f. A(x) satisfies A(x) = 1 + x*A(x*A(x))^3.
b(n, k) = my(A=1); for(i=1, n, A = 1 + x * subst(A, x, x*A)^3 + x*O(x^n)); polcoef(A^k, n);
for(n=0, 10, for(k=0, 15, print1(a(n, k)-b(n, k),", ")); print);

