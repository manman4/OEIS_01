\\ T(n,k) = [x^k] F_n(x), for n >= 0, k = 1..2^n,
\\ using
\\   F_n(x) = x * prod(j = 0, n - 1, 1 + F_j(x)),
\\ with F_0(x) = x.

Fn(n) =
{
  my(x = 'x, m = 2^n, P = 1 + x * O(x^m), F = x + x * O(x^m));
  for(j = 0, n - 1,
    P = P * (1 + F);
    F = x * P;
  );
  F;
}

Trow(n) =
{
  my(m = 2^n, F = Fn(n));
  vector(m, k, polcoeff(F, k));
}

T(n, k) =
{
  if(k < 1 || k > 2^n, error("1 <= k <= 2^n must hold"));
  Trow(n)[k];
}

triangle(N) =
{
  my(v = []);
  for(n = 0, N, v = concat(v, Trow(n)));
  v;
}

write_triangle_data(N, filename) =
{
  my(v = triangle(N));
  system(Str("rm -f ", filename));
  for(i = 1, #v,
    write(filename, Str(i - 1, " ", v[i]))
  );
}

\\ example
N = 10;
write_triangle_data(N, "b122888_2.txt");
