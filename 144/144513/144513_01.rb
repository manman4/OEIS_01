def A144513(n)
  a, b = 2, 18
  cnt = 0
  ary = [2]
  while cnt < n
    cnt += 1
    a, b = b, ((2 * cnt + 3) * (cnt ** 2 + 3 * cnt + 3) * b + (cnt + 2) ** 2 * a) / ((cnt + 1) ** 2)
    ary << a
  end
  ary
end


n = 2000
ary = A144513(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
