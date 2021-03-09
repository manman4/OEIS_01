n = 300
(0..n).each{|i|
  j = 111111 ** i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
