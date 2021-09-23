def A(n)
  ary = [0, 1]
  a, b = 1, 3
  (2..n).each{|i|
    a, b = b, 3 * b + i ** 2 * a
    ary << a
  }
  ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
