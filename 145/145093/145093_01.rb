def A(n)
  ary = [1]
  a, b = -720, 269280
  s = 3
  while (s - 3) < n
    ary << a
    c = -12 * (2 * (144 * s * s - 288 * s + 115) * b / (2 * s - 1r) + 3 * (12 * s - 25) * (12 * s - 19) * (12 * s - 17) * (12 * s - 11) * a / ((s - 1r) * (s - 2) * (2 * s - 3))) / (2 * s - 3r)
    c = c.to_i if c.denominator == 1
    a, b = b, c
    s += 1
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
