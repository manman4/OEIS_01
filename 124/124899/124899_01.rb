(1..500).each{|i|
  j = ((2 * i - 1) ** (2 * i - 1) + 1) / (2 * i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
