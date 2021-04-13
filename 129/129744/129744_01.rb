def A(n)
  a, b, c, d = 2, 4, 14, 32
  ary = [2]
  while ary.size < n
    a, b, c, d = b, c, d, 2 * d + 2 * c - 2 * b - a
    ary << a
  end
  ary[0..n - 1]
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
