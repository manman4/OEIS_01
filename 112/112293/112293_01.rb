n = 500
s = 1
(0..n).each{|i|
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
  s *= 2 * i + 1
  s += 1
}
