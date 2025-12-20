def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..floor(n/2)} binomial(2n-2k,2k)3^k*2^(n-k).
def A(n)
  (0..n / 2).inject(0){|s, i| s + ncr(2 * n - 2 * i, 2 * i) * 3 ** i * 2 ** (n - i)}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
