def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n / 2).inject(0){|s, i| s + n ** (n - i) * ncr(n - i, i)}
end

n = 1000
(1..n).each{|i|
  j = A(i - 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
