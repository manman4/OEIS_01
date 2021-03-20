def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A1(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(3, i) + s(5, i) + s(15, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A2(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, i) + s(6, i) + s(10, i) + s(30, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary1 = [0] * 1 + A1(n)
ary2 = [0] * 2 + A2(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary1[i] + 2 * ary2[i]
}
