def A(n)
  return 1 if n == 0
  (n + 1..2 * n).inject(:*)
end

n = 1000
(0..n).each{|i|
  j = A(i) ** 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
