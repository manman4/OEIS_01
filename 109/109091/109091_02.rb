def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)
}
