def A(n)
  ary = []
  a, b, c = 1, 1, 1
  cnt = 0
  while cnt <= n
    cnt += 1
    d = c + (cnt - 2) * (cnt - 1) * cnt * a
    a, b, c = b, c, d
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