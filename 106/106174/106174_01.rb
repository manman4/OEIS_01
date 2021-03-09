def A106174(n)
  a, b = 0, 1
  ary = [0]
  i = 0
  while i < n
    a, b = b, (2 * i + 4) * b - a
    ary << a
    i += 1
  end
  ary
end

n = 500
ary = A106174(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
