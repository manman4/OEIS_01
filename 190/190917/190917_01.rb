def A(n)
  ary = [1, 5]
  s, t = 1, 5
  i = 3
  while i <= n
    s, t = t, ((i + 1) * (7 * i - 4) * t + 8 * (i - 2) ** 2 * s) / (i * (i + 1))
    ary << t
    i += 1
  end
  ary
end

n = 1200
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
