def A(k, n)
  a = [1]
  ary = [1]
  n.times{|i|
    a << 0
    b = [0]
    (0..i).each{|j|
      b[j + 1] = a[j] + (j + 1) * a[j + 1]
    }
    a = b
    # k * (1/k) * (-1) ** (i + 1) * a[0] = (-1) ** (i + 1) * 0 = 0
    ary << (1..i + 1).inject(0){|s, j| s + k * (j + k) ** (j - 1) * (-1) ** (i + 1 - j) * a[j]}
  }
  ary
end

n = 500
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
