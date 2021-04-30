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

def A145493(n)
  (0..n).map{|i| f(6 * i + 6) * (41 * i + 77) / (2310 * f(3 * i + 3) * f(i) * f(i + 2) ** 2)}
end

n = 10
p ary = A145493(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
