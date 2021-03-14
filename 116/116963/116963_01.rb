def A(n)
  s = 0
  (1..n).each{|i| s += (i + 1) * (i + 2) * (i + 3) / 6 if n % i == 0}
  s
end

def A116963(n)
  (1..n).map{|i| A(i)}
end

n = 10000
ary = A116963(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
