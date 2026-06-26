\\ T(n,k) = Sum_{0 = x_0 <= x_1 <= ... <= x_{n-1} <= x_n = k - 1}
\\          Product_{j=0..n-1} binomial(x_j + 1, x_{j+1} - x_j),
\\ for n >= 0, k = 1..2^n.
\\
\\ This gives the recurrence
\\   T(0,1) = 1,
\\   T(n,k) = sum(j = 1, k, T(n-1,j) * binomial(j, k - j)).

Trow(n) =
{
  if(n == 0, return([1]));
  my(prev = [1], curr, m);
  for(step = 1, n,
    m = 2^step;
    curr = vector(m, k,
      my(s = 0);
      for(j = 1, min(k, #prev),
        s += prev[j] * binomial(j, k - j)
      );
      s
    );
    prev = curr;
  );
  prev;
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

write_triangle_data(N) =
{
  my(v = triangle(N), fh);
  fh = fileopen("b122888.txt", "w");
  for(i = 1, #v,
    filewrite(fh, Str(i - 1, " ", v[i]))
  );
  fileclose(fh);
}

\\ example
N = 10;
write_triangle_data(N);
