def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} C(2k+1,k)*C(2k+1,n-k)
def A(n)
  return 1 if n == 0
  (0..n).inject(0){|s, i| s + ncr(2 * i + 1, i) * ncr(2 * i + 1, n - i)}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}