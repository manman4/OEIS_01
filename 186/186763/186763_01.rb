def A186763(n)
  a = 0
  ary = (0..n).map{|i| a = -i * a + i % 2}
  ary.map{|i| i.abs}
end

n = 500
ary = A186763(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
