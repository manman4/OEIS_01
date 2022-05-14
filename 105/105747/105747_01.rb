def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 0
  (0..n).each{|i|
    (i..n).each{|j|
      # f(i + j)/f(j), f(j)/(f(i) * f(j - i))が整数のなので、積も整数
      s += f(i + j) / (f(i) * f(j - i))
    }
  }
  s.to_i
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}