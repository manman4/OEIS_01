def A(n)
  a, b = 0, 4
  ary = [0]
  while ary.size <= n
    a, b = b, 18 * b - a + 8
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
