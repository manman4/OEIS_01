(0..8192).each{|i| 
  print i
  print ' '
  puts (i.to_s(4).split('').sort.reverse.join.to_i(4) - i.to_s(4).split('').sort.join.to_i(4)) / 3
}
