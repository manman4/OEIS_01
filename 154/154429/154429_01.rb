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

def C(n)
  m = 24 * n + 1r
  s = 1
  while B(4 * s / m).size > 3
    s += 1
  end
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts C(i)
}
