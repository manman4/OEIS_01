def A(n)
  ary = [1]
  a, b, c = 1, 1, 5
  cnt = 0
  while cnt < n
    cnt += 1
    a, b, c = b, c, (2 * cnt + 3) * c - (cnt - 2)*(cnt + 1) * b - cnt * (cnt + 1) * a
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
