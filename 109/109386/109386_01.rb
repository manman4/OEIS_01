def A001227(n)
  1.step(n, 2).select{|i| n % i == 0}.size
end

def s(n)
  s = 0
  (1..n).each{|i| s += i * A001227(i) if n % i == 0}
  s
end

def A(n)
  (1..n).map{|i| s(i)}
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
