def A(m, n)
  a, b = 1, 1
  ary = [1]
  cnt = 0
  while cnt < n
    a, b = b, b - m * a
    ary << a
    cnt += 1
  end
  ary
end

def A145934(n)
  A(6, n)
end

n = 10000
ary = A145934(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
