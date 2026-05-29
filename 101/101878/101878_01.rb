def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} (k+1)^k * (n+1)^(n-k-1) * binomial(n, k).
def a(n)
  (0..n).inject(0){|s, k| s + (k + 1)**k * (n + 1)**(n - k - 1) * ncr(n, k)}.to_i
end

p (0..17).map{|i| a(i)}