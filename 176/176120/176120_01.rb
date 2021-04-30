def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << (f(i - 1) * (1..i).inject(0){|s, j| s + (j + k) * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end
def A176120(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i - 1).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end
n = 140
ary = A176120(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
