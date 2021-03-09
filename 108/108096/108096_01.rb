def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % 2 == 1}
  s
end

# ary[0] = 1
def sqrt_a(ary)
  n = ary.size - 1
  a = [1]
  (0..n - 1).each{|i| a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2}
  a
end

def A(n)
  ary = [1] + (1..n).map{|i| 24 * s(i)}
  sqrt_a(ary)
end

n = 100
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
