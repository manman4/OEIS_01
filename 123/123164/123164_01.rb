def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  return 1 if n == 0
  (0..n).inject(0){|s, i| s + ncr(n, i) * ncr(k * n + i - 1, n - 1)} 
end

n = 1000
(0..n).each{|i|
  j = A(i, 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
