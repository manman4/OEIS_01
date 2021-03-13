def A(n)
  ary = [2]
  a, b = 2, 7
  (n - 1).times{
    c = b / 2
    ary << c
    a = c
    b += a
  }
  ary
end

n = 6000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
