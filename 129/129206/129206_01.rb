def A(u, v, w, x, n)
  a, b, c, d = u, v, w, x
  ary = [u]
  cnt = 0
  while cnt < n
    i = 144 * b * d + 2257 * c * c
    break if i % a > 0
    a, b, c, d = b, c, d, i / a
    ary << a
    cnt += 1
  end
  ary
end

n = 100
ary = [0] + A(1, -12, -2257, 1494696, n - 1)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
