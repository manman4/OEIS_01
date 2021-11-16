def A(n)
  n.to_s.split('').inject(0){|s, i| s + i.to_i}
end

def B(n)
  ary = [0, 1]
  s = 1
  t = 1
  (2..n).each{|i|
    s *= (i - 1)
    t += s
    ary << A(t)
  }
  ary
end

n = 58
p ary = B(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
