def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts (ncr(3 * i, i) / (2 * i + 1)) % 3
}
