require 'prime'

def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def f(n)
  return 1 if n < 2
  (1..n).map{|i| 2 * i - 1}.inject(:*)
end

def B(n)
  ary0 = Array.new(n + 1, 1)
  ary1 = [0, 3, -2, -1, -1, -2, 3]
  ary2 = (0..n).map{|i| ary1[i % 7]}
  A(ary2, ary0, n)
end

def I(ary, n)
  a = [1]
  i = 0
  while i < n
    a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}
    i += 1
  end
  a
end

n = 100
p ary = B(n + 1)
b = I(ary, n)
ary1 = ary.clone
ary1[1] = 2
p ary1
p c = [1] + I(ary1, n)
p [1, 3] + (0..n - 2).map{|i| ary[i + 2] - b[i] - c[i]}

