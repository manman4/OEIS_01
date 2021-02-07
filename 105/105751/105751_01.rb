def A(n)
  a = 1
  (1..n).each{|i|
    a *= 1 + i * 1i
  }
  a.imaginary
end

n = 500
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
