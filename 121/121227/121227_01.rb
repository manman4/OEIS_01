def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..2*n} binomial(k*(k+1)/2+n-1, n)*(Sum_{r=k..2*n} binomial(r, k)*(-1)^(r-k)).
def A(n)
  (0..2 * n).inject(0){|s, k| s + ncr(k * (k + 1) / 2 + n - 1, n) * (k..2 * n).inject(0){|t, r| t + ncr(r, k) * (-1) ** (r - k)}}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
