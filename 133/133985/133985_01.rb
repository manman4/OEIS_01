def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A133985(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(4, i) + 5 * s(6, i) - 2 * s(2, i) - 2 * s(3, i) - 2 * s(12, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary = A133985(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
