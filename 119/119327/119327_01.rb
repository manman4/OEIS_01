def A(n)
  a, b, c, d, e = 1, 1, 7, 19, 38
  ary = [1]
  while ary.size <= n
    a, b, c, d, e = b, c, d, e, 5 * e - 10 * d + 10 * c - 5 * b + a
    ary << a
  end
  ary
end

n = 10000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
