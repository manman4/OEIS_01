def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && i % k == m
  }
  s
end
def A104409(n)
  ary = [1]
  a = [0] + (1..n).map{|i| -A(14, 4, i) + A(14, 5, i) - A(14, 6, i) + A(14, 7, i) - A(14, 8, i) + A(14, 9, i) - A(14, 10, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

m = 10100
ary = A104409(m)
(0..10000).each{|i|
  print i
  print ' '
  puts ary[i]
}