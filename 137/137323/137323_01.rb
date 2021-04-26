def s(n)
  s = 0
  (2..n - 1).each{|i| s += 2 ** (n / i - 1) - 1 if n % i == 0}
  s
end

n = 10000
(1..n).each{|i|
  j = s(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
