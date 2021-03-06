def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A176997(n)
  ary = []
  i = 1
  while ary.size < n
    ary << i if pow(2, i - 1, i) == pow(2, 0, i)
    i += 1
  end
  ary
end

n = 10000
ary = A176997(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
