def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 2000
print 0
print ' '
puts 1
(1..n).each{|i|
  j = -ncr(2 * (i - 1), i - 1) / i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
