def A(n)
  ary = [1, 84]
  s = 84
  (2..n).each{|i|
    a = 6 * i + (-1) ** ((i - 1) % 2)
    s *= 12 * a * (a - 6)
    s /= (i * (i - 1))
    ary << s
  }
  ary[0..n]
end

n = 20
p ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
