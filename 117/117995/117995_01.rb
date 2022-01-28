# m>0

def s(f_ary, g_ary, n, m)
  s = 0
  (m..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n, m = 1)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i, m)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

n = 10000
ary1 = Array.new(n + 1, 1)

# Product_{k>1} 1/(1 - x^k)
ary = A([[ary1, ary1]], n, 2)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i] - (i + 1) % 2
}
