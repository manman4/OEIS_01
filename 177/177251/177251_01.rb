def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n, k)
  (0..n / k).inject(0){|s, i| s + (-1) ** i * f(n - (k - 1) * i) / f(i)}
end

n = 1000
(0..n).each{|i|
  j = A(i, 3)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
