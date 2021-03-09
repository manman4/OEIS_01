def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def A004009(n)
  a = [1] + (1..n).map{|i| 240 * A001158(i)}
end

def E4(n)
  ary = A004009(n)
end

# ary[0] = 1, ary[1] �͋���
def sqrt_a(ary)
  n = ary.size - 1
  a = [1, ary[1] / 2]
  i = 1
  while i < n
    a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2
    i += 1
  end
  a
end

def A108091(n)
  a0 = E4(n)
  a1 = sqrt_a(a0)
  a2 = sqrt_a(a1)
  sqrt_a(a2)
end

n = 500
ary = A108091(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
