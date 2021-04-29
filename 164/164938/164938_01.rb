n = 10000
(1..n).each{|i|
  j = (i ** 5 - i) / 10
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
