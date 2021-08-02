def A(n)
  (1..n - 1).inject(0){|s, i| s + i ** n}
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
