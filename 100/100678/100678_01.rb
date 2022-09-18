def A(x, y)
  m = y / x
  m += 1 if y % x > 0
  m
end

def B(n, a = [])
  return [1] if n == 1
  x, y = n.numerator, n.denominator
  return a if y == 1
  m = A(x, y)
  a << m
  x, y = (-y) % x, y * m
  n = x.to_r / y
  return B(n, a)
end

n = 10000
(1..n).each{|i|
  j = B(i.to_r / (i + 1)).size
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
