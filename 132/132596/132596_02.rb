def A(n)
  a, b = 0, 2
  ary = [0]
  while ary.size <= n
    a, b = b, 10 * b - a + 4
    ary << a
  end
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
