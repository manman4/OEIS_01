# バイナリ法
def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def A(n)
  ns = n.to_s(2).size
  s = 2
  while power(n, s).to_s(2)[0..ns - 1] != n.to_s(2)
    s += 1
    p s
  end
  s
end

# p A(639)
(1..500).each{|i|
  print i
  print ' '
  puts A(i)
}
