(0..10000).each{|i| 
  print i
  print ' '
  puts (i.to_s(10).split('').sort.reverse.join.to_i(10) - i.to_s(10).split('').sort.join.to_i(10)) / 9
}
