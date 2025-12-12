def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{i=0..n} binomial(2n+i,2i).
def a(n)
  (0..n).inject(0){|s, i| s + ncr(2 * n + i, 2 * i)}
end

n = 1000
(0..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
  