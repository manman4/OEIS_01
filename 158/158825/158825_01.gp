\\ A(n,k) = B(k-1,n), where B(n,k) = Sum_{0 = x_0 <= x_1 <= ... <= x_{k-1} <= x_k = n} Product_{j=0..k-1} (x_j + 1) * binomial(2*x_{j+1} - x_j + 1,x_{j+1} - x_j)/(2*x_{j+1} - x_j + 1).
b(n, k, m=2) = {
  my(T=matrix(n+1, n+1, r, c, my(xr=r-1, xc=c-1); if(xc<xr, 0, (xr+1)*binomial(m*xc-(m-1)*xr+1, xc-xr)/(m*xc-(m-1)*xr+1))));
  my(TK=T^k);
  TK[1, n+1];
};
a(n, k) = b(k-1, n);
for(k=1, 15, print1(a(1, k),", "));
for(k=1, 15, print1(a(2, k),", "));
for(k=1, 15, print1(a(3, k),", "));
for(k=1, 15, print1(a(4, k),", "));
for(k=1, 15, print1(a(5, k),", "));

matrix(7, 7, n, k, a(n, k))

