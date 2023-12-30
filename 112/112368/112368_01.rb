def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 1000
s = 0
(0..n).each{|i|
  s += 2 ** i * f(i)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
