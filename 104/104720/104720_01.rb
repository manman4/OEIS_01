n = 1000
s = 0
t = 0
(0..n).each{|i|
  t = (t.to_s + ((i + 1) % 2).to_s).to_i
  s += t
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
