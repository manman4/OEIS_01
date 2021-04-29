def A028942(n)
  ary = [1]
  a, b = 1r, 0r
  (n - 1).times{
    ary << a.denominator
    c = (b * b - a * a * a) / (a * a)
    d = -1 - b * c / a
    a, b = c, d
  }
  ary
end

def A028945(n)
  return [1] if n == 0
  ary = A028942(2 * n - 3)
  [1, 1] + (0..n - 2).map{|i| ary[2 * i]}
end

def A151502(n)
  ary = A028945(n)
  (0..n).map{|i| ary[i] ** 2}
end

n = 80
ary = A151502(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
