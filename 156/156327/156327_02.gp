M=17;

\\ n階微分
diff_n(f, x, n) = {
  my(res=f);
  for (i=1, n, res = deriv(res, x));
  return(res);
}

\\ E.g.f. A(x) satisfies A(x) = exp(2 * x * A(x) + x^2/2 * A'(x)). 
my(A=1, n=M); for(i=1, n, A= exp(2 * x * A + x^2/2 * diff_n(A, x, 1) + x*O(x^n)) ); Vec(serlaplace(A))

my(A=1, n=M); for(i=1, n, A= exp(2 * x * A + x^2/2 * diff_n(A, x, 1) + x*O(x^n)) ); Vec(serlaplace(exp(2 * x * A + x^2/2 * diff_n(A, x, 1) + x*O(x^n))))
