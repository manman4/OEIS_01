def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

n = 10000
ary = [0] + (1..n).map{|i| s(1, i) - 5 * s(2, i) / 2 + s(4, i)}
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
