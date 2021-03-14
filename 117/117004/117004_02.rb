def A(n)
  d = 0
  s = 0
  (1..n).each{|i|
    if n % i == 0
      d += i
      s += (n / i - i).abs
    end
  }
  d - s / 2
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
