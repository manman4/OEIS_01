\\ A(n,k) = n! * [x^n] F_k(x)/x and F_k(x) is the k-th iteration of x*G(x) with G(x) = exp(x*G(x)).
Egen(n,k) = {
  my(N=n+1, x = 'x + O('x^(N+1)));
  my(g=1 + sum(j = 1, N, (j+1)^(j-1)/j! * x^j));
  \\ G(x) = exp(x*G(x))の確認
  if(g!=exp(x*g), print("G(x) is wrong"));
  my(F_iter=x);

  for(i=1, k, F_iter=subst(x*g, 'x, F_iter));

  n! * polcoef(F_iter/x, n)
};

for(n=0, 19, print1(Egen(n, 2),", "));

\\ E.g.f. of column k: (1/x) * Series_Reversion( H_k(x) ), where H_k(x) is the k-th iterate of x*exp(-x).
Tgen(n,k) = {
  my(N=n+1, x = 'x + O('x^(N+1)));
  \\ U=-LambertW(-(p-s)*x)/(p-s), or U = x when p = s
  my(Finv=x * exp(-x));
  my(Finv_k=x);

  for(i=1, k, Finv_k=subst(Finv, 'x, Finv_k));

  n! * polcoef(serreverse(Finv_k)/x, n)
};

for(n=0, 19, print1(Tgen(n, 2)-Egen(n, 2),", "));


\\ E.g.f. of column k: -W_k(-x)/x, where W_k(x) is the k-th iterate of LambertW(x).
T1(n,k) = {
  my(N=n+1, x = 'x + O('x^(N+1)));
  my(Finv=lambertw(x));
  my(Finv_k=x);

  for(i=1, k, Finv_k = subst(Finv, 'x, Finv_k));

  n! * polcoef(-subst(Finv_k, 'x, -x)/x, n)
};

for(n=0, 19, print1(T1(n, 2)-Egen(n, 2),", "));


\\ E.g.f.: (1/x) * Series_Reversion( H_2(x) ), where H_k(x) is the k-th iterate of x*exp(-x). 
my(N=20, x='x + O('x^(N+1)));
phi = x*exp(-x);
Vec(serlaplace(serreverse(subst(phi, 'x, subst(phi, 'x, x))) / x))
