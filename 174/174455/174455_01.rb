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

n = 10100
m = 10000
ary1 = Array.new(n + 1, 1)
ary1[3] = 2
ary2 = [0, 0, 0] + [1] * (n - 2)
ary = A(ary1, ary2, n)
(0..m).each{|i|
  print i
  print ' '
  puts ary[i]
}