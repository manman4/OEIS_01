n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts ((i * (i + 1)) ** 2 / 4) % i
}
