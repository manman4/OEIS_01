n = 10000
m = 2 ** 7 - 2 ** 3
(0..n).each{|i|
  print i
  print ' '
  puts ((i + 2) ** 7 - (i + 2) ** 3) / m
}
