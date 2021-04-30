def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 4 * s(1, i) + 9 * s(6, i) - 3 * s(2, i) - 4 * s(3, i) - s(4, i) - 5 * s(12, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary = A(n + 1)
ary[1] = 4
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
