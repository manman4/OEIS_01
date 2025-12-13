def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = binomial(3*n+1,n+1)-binomial(2*n,n+1).
def a(n)
  ncr(3 * n + 1, n + 1) - ncr(2 * n, n + 1)
end

n = 1000
(0..n).each{|i| 
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
} 