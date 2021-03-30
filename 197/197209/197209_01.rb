def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  ncr(n - 1, k - 1) * ncr(n, k + 1) * ncr(n + 1, k)
end

def B(n)
  (1..n - 1).inject(0){|s, i| s + A(n, i)}
end

n = 1000
(2..n).each{|i|
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}