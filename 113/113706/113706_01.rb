def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A113706(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(3, 1, i) - s(3, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + ary[-j] * s_ary[j]} / i}
  ary
end

n = 10000
ary = A113706(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
