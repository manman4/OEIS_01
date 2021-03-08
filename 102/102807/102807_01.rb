n = 1000
(0..n).each{|i|
  j = (100 ** i + 4 * 10 ** i + 4) / 9
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
