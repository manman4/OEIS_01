def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  (-1) ** (k % 2) * (0..n).inject(0){|s, i| s + f(n + k + i) / (f(n - i) * f(i) * 2 ** i)} / f(k)
end

def B(k, n)
  (0..n).map{|i| A(k, i)}
end

def A144505(n)
  a = []
  (0..n - 1).each{|i| a << B(i, n - i)}
  ary = [1]
  (0..n - 1).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 140
ary = A144505(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
