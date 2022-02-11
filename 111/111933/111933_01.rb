def f(n)
  return 1 if n < 2
  return (1..n).inject(:*)
end

def A(ary, n)
  a = [1]
  ary1 = [0]
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (i - 1) * a[j + 1]
    }
    a = b
    ary1 << (0..i).inject(0){|s, j| s + ary[j] * a[j]}
  }
  ary1
end

def B(k, n)
  ary = [0] + (1..n).map{|i| f(i - 1)}
  (k - 1).times{ary = A(ary, n)}
  ary[1..-1]
end

def A111933(n)
  a = []
  (1..n).each{|i| a << B(i, n - i + 1)}
  ary = []
  (0..n - 1).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 140
ary = A111933(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
