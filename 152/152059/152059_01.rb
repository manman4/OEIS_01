def A(k, n)
  ary = [1]
  a, b = 1, k + 1
  (1..n).each{|i|
    a, b = b, (2 * i + 1 + k) * b - i * (i - 1 + k) * a
    ary << a
  }
  ary
end

n = 500
(0..n).each{|i|
  j = A(i, i)[-1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
