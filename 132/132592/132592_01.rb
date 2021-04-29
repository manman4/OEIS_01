def A(n)
  a, b = 0, 8
  ary = [0]
  while ary.size <= n
    a, b = b, 34 * b - a + 16
    ary << a
  end
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
