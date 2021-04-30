def A000166(n)
  ary = [1]
  (1..n).each{|i| ary << i * ary[-1] + (-1) ** (i % 2)}
  ary
end

n = 500
ary = A000166(n - 1)
(1..n).each{|i|
  j = (-1) ** (i % 2 - 1) * ary[i - 1] + 1
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
