def A(k, n)
  s = 0
  (1..n).each{|i|
    s += [i, n / i].min ** k if n % i == 0
  }
  s
end

n = 100
(1..n).each{|i|
  print i
  print ' '
  puts A(1, i)
}
