def A(n)
  (1..n).map{|i| (1..i).inject(0){|s, j| s - (-1) ** (j % 2) * i.gcd(j)}}
end

n = 100
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
