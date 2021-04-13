def A(n)
  a = [0, 0, 0, 0]
  b = [1]
  ary = []
  (2 * n + 1).times{|i|
    ary << b[i / 2] if i % 2 == 0
    b = ([0, 0] + b + [0, 0])
    a, b = b, (0..i + 1).map{|j| a[j] + a[j + 1] + a[j + 2] + b[j + 1] + b[j + 2]}
  }
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
