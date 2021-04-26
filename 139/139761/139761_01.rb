def A(n)
  a, b, c, d, e = 0, 0, 0, 0, 1
  ary = [0]
  while ary.size <= n
    a, b, c, d, e = b, c, d, e, 5 * e - 10 * d + 10 * c - 5 * b + 2 * a
    ary << a
  end
  ary
end

n = 3000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
