def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (1..n).map{|i| (1..i).inject(0){|s, j| s + ncr(i, j) * i.gcd(j)}}
end

n = 1000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
