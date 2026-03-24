N=15;

a(n) = (n+1)^(2*n) * (1/(n+1)! - sum(k=0, n-1, ((k+1)/(n+1)^(k+1))^2 * a(k)/(n-k)!));
for(n=0, N, print1(a(n), ", "));
\\ 1/(n+1)! = Sum_{k=0..n} ((k+1)/(n+1)^(k+1))^2 * a(k)/(n-k)!.
for(n=0, N, print1(sum(k=0, n, ((k+1)/(n+1)^(k+1))^2 * a(k)/(n-k)!) - 1/(n+1)!, ", "));

\\ a(0), ... a(N)
b(n) = my(x='x + O(x^(N+5)), A = sum(k=0, N, a(k)*x^k)); polcoef(exp(n^2*x) * (1 - x*deriv(x*deriv(x*A))), n);
for(n=0, N, print1(b(n), ", "));

a(n) = (n+1)^(3*n) * (1/(n+1)! - sum(k=0, n-1, ((k+1)/(n+1)^(k+1))^3 * a(k)/(n-k)!));
for(n=0, N, print1(a(n), ", "));
\\ 1/(n+1)! = Sum_{k=0..n} ((k+1)/(n+1)^(k+1))^3 * a(k)/(n-k)!.
for(n=0, N, print1(sum(k=0, n, ((k+1)/(n+1)^(k+1))^3 * a(k)/(n-k)!) - 1/(n+1)!, ", "));

\\ a(0), ... a(N)
b(n) = my(x='x + O(x^(N+5)), A = sum(k=0, N, a(k)*x^k)); polcoef(exp(n^3*x) * (1 - x*deriv(x*deriv(x*deriv(x*A)))), n);
for(n=0, N, print1(b(n), ", "));

a(n) = (n+1)^(4*n) * (1/(n+1)! - sum(k=0, n-1, ((k+1)/(n+1)^(k+1))^4 * a(k)/(n-k)!));
for(n=0, N, print1(a(n), ", "));
\\ 1/(n+1)! = Sum_{k=0..n} ((k+1)/(n+1)^(k+1))^4 * a(k)/(n-k)!.
for(n=0, N, print1(sum(k=0, n, ((k+1)/(n+1)^(k+1))^4 * a(k)/(n-k)!) - 1/(n+1)!, ", "));

\\ a(0), ... a(N)
b(n) = my(x='x + O(x^(N+5)), A = sum(k=0, N, a(k)*x^k)); polcoef(exp(n^4*x) * (1 - x*deriv(x*deriv(x*deriv(x*deriv(x*A))))), n);
for(n=0, N, print1(b(n), ", "));

a(n) = (n+1)^(5*n) * (1/(n+1)! - sum(k=0, n-1, ((k+1)/(n+1)^(k+1))^5 * a(k)/(n-k)!));
for(n=0, N, print1(a(n), ", "));
\\ 1/(n+1)! = Sum_{k=0..n} ((k+1)/(n+1)^(k+1))^5 * a(k)/(n-k)!.
for(n=0, N, print1(sum(k=0, n, ((k+1)/(n+1)^(k+1))^5 * a(k)/(n-k)!) - 1/(n+1)!, ", "));

\\ a(0), ... a(N)
b(n) = my(x='x + O(x^(N+5)), A = sum(k=0, N, a(k)*x^k)); polcoef(exp(n^5*x) * (1 - x*deriv(x*deriv(x*deriv(x*deriv(x*deriv(x*A)))))), n);
for(n=0, N, print1(b(n), ", "));
