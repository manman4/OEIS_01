def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  (0..n / k).inject(0){|s, i| s + ncr(n, k * i + k - 1)}
end

def B(n)
  (0..n).inject(0){|s, i| s + A(n - i + 1, i)}
end

n = 500
(0..n).each{|i|
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
