def A125143(n)
  a, b = 1, -3
  ary = [1]
  i = 0
  while i < n
    a, b = b, (- (2 * i + 3) * (7 * i * i + 21 * i + 17) * b - 81 * (i + 1) ** 3 * a) / (i + 2) ** 3
    ary << a
    i += 1
  end
  ary
end

n = 1100
ary = A125143(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
