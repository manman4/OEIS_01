(0..10000).each{|i|
  print i
  print ' '
  puts i.to_s.split('').sort.reverse.join.to_i - i
}
