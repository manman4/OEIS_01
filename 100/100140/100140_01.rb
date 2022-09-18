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
  ary = []
  (2..n).each{|i|
    max = 0
    (1..i - 1).each{|j|
      k = B(j.to_r / i)[-1]
      max = k if max < k
    }
    ary << max
  }
  ary
end

n = 200
ary = C(n)
(2..n).each{|i|
  j = ary[i - 2]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
