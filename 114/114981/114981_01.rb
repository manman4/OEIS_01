def pow(a, m, mod)
  return 1 % mod if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A(k, n)
  ary = []
  i = 1
  while ary.size < n
    mod = i + k
    ary << i if (pow(k, i, mod) + pow(i, k, mod)) % mod == 0
    i += 1
  end
  ary
end

n = 500
ary = A(6, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
