def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} 3^k * binomial(2n, n-k).
def A100193(n)
  (0..n).inject(0){|s, i| s + 3 ** i * ncr(2 * n, n - i)}
end

n = 1000
(0..n).each{|i|
  j = A100193(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}