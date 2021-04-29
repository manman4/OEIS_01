def A(n)
  a, b = 0, 1
  cnt = 0
  ary = [0]
  while ary.size <= n
    cnt += 1
    a, b = b, a + b + 1 + (-1) ** (cnt % 2)
    ary << a
  end
  ary
end

n = 2000
ary = A(n)
(0..n).each{|i|
  j = ary[i] ** 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
