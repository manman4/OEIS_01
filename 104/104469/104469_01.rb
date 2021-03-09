def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == m || i % k == k - m)
  }
  s
end
def A104469(n)
  ary = [1]
  a = [0] + (1..n).map{|i| A(9, 1, i) - A(9, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

m = 10100
ary = A104469(m)
(0..10000).each{|i|
  print i
  print ' '
  puts ary[i]
}