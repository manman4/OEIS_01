def d(ary)
  s = ary.size - 1
  return [0] if s == 0
  (1..s).map{|i| i * ary[i]}
end

def e(ary)
  while ary[-1] == 0
    ary.pop
  end
  ary
end

def A(n)
  a, b = [0], [1]
  ary = [0]
  cnt = -1
  while cnt < n
    cnt += 1
    s, t = a.size - 1, b.size - 1
    c0 = d(b)
    c = [0] * 3 + c0.map{|i| i * 8}
    (0..t - 1).each{|i| c[i] -= c0[i]}
    u = 16 * cnt + 3
    (0..t).each{|i| c[i + 2] -= b[i] * u}
    v = 4 * cnt * (2 * cnt - 1)
    (0..s).each{|i| c[i + 1] -= a[i] * v}
    a, b = b, e(c)
    ary += a
  end
  ary
end

ary = A(99)
(-1..ary.size - 2).each{|i|
  print i
  print ' '
  puts ary[i + 1]
}
