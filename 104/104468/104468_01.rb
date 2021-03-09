def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == m || i % k == k - m)
  }
  s
end
def A104468(n)
  ary = [1]
  a = [0] + (1..n).map{|i| A(9, 2, i) - A(9, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

m = 10100
n = 10000
ary = A104468(m)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}