def A(n)
  ary = []
  a, b, c, d = 1, 1, 1, 1
  cnt = 0
  while cnt <= n
    cnt += 1
    e = d + (cnt - 2) * (cnt - 1) * cnt * (cnt + 1)  * a
    a, b, c, d = b, c, d, e
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