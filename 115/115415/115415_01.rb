n = 500
(0..n).each{|i|
  j = ((i + 1i) ** i).real
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
