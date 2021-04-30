def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 8 * n + 7
  s *= f(6 * n + 1)
  s /= f(3 * n)
  s /= f(n)
  s /= (f(n + 1)) ** 2
  s
end

n = 500
(0..n).each{|i|
  j = A(i)
  break if j % 7 > 0
  j /= 7
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
