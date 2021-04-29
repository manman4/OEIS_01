def t(n)
  return 0 if n == 0
  m = n.to_s(2).size - 1
  n1 = 2 ** m
  i = n - n1
  t = (n1 * n1 * 2 + 1) / 3
  return t if i == 0
  return t + t(i) * 2 + t(i + 1) - 1
end


(0..2 ** 13 - 1).each{|i|
  print i
  print ' '
  puts (t(i + 1) - 1) / 2
}
