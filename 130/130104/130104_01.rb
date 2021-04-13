def A130104(n)
  ary = [0, 1]
  (1..n - 1).each{|i| ary << 2 * ary[i] + (i - 1) * (i + 2) / 2}
  ary[0..n]
end

n = 10000
ary = A130104(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
