def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i if n % i == 0 && i % k == m}
  s
end
def A109506(n)
  (1..n).map{|i| s(2, 1, i) - s(2, 0, i)}
end

n = 10000
ary = A109506(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
