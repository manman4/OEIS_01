n = 600
(1..n).each{|i|
  j = ([4] * i + [8] * (i - 1) + [9]).join.to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
