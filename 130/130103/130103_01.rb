def A130103(n)
  ary = [0, 1, 1]
  (2..n - 1).each{|i| ary << 2 * ary[i] + i}
  ary[0..n]
end

n = 10000
ary = A130103(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
