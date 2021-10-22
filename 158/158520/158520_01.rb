def a(n)
  cnt = 1
  i = 0
  s = 1
  print 1
  print ' '
  puts 0
  while cnt < n
    i += 1
    s *= 2
    t = s.to_s.size
    if s % t == 0
      cnt +=1
      print cnt
      print ' '
      puts i
    end
  end
end

n = 49
a(n)

