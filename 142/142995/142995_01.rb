def A(n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << (2 * i * i - 2 * i + 3) * ary[-1] - (i - 1) ** 4 * ary[-2]}
  ary
end

n = 300
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
