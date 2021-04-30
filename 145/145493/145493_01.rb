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

def A001421(n)
  (0..n).map{|i| f(6 * i) / (f(3 * i) * f(i) ** 3)}
end

def B(n)
  (0..n).map{|i| A(i) * 12}
end

def C(n)
  a = A001421(n + 1)
  b = B(n + 1)
  (1..n + 1).map{|i| b[i] - 84 * a[i]}
end

n = 400
ary = C(n)
(0..n).each{|i|
  j = ary[i]
  break if j % 27720 > 0
  j /= 27720
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
