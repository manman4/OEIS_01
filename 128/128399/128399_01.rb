def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A128399(n)
  cnt = 0
  i = 1
  while cnt < n
    j = pow(19, i, i * i)
    if j == 1 % i
      cnt += 1
      print cnt
      print ' '
      puts i
    end
    i += 1
  end
end

n = 50
A128399(n)
