def f(k, m, n)
  a, b = 1, 2 * m
  ary = [1]
  cnt = 0
  while cnt < n
    a, b = b, (2 * m * b + k * k * cnt * a) / (cnt + 2r)
    a = a.to_i if a.denominator == 1
    ary << a
    cnt += 1
  end
  ary
end

def A(k, n)
  a = f(1, k, 2 * n)
  ary = []
  s = 0
  (0..n).each{|i|
    s += a[2 * i]
    ary << s
  }
  ary
end

def A142992(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 139
ary = A142992(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
