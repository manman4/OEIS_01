def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# B(n,k) = Sum_{0 = x_0 <= x_1 <= ... <= x_{k-1} <= x_k = n} Product_{j=0..k-1} (x_j + 1) * binomial(2*x_{j+1} - x_j + 1,x_{j+1} - x_j)/(2*x_{j+1} - x_j + 1).
# x_0 = 0, x_k = nに注意
def b(n, k)
  return n == 0 ? 1 : 0 if k == 0
  sum = 0
  (0..n).each{|x|
    sum += b(x, k-1) * (x + 1) * ncr(2*n - x + 1, n - x) / (2*n - x + 1r)
  }
  sum.to_i
end

# A(n,k) = B(k-1,n)
def a(n, k)
  b(k - 1, n)
end

n = 12
(1..n).each{|i|
  (1..i - 1).each{|j|
    print a(i - j, j)
    print ', '
  }
}
