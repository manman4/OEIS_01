def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 400
(1..n).each{|i|
  j = (i + 1) ** i - f(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
