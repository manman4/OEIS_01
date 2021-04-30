n = 3000
(0..n).each{|i|
  j = 2 ** i + 2 * i + 1
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
