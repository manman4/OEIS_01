def A(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    b = a + [0] * (i * i)
    (0..a.size - 1).each{|j| b[i * i + j] += a[j]}
    a = b
    ary << a.max
  }
  ary
end

n = 210
ary = A(n)
m = 200
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
