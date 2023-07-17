# a(n) = n*sumdiv(n, d, (-1)^(n+d)*(d+1)*(n/d+1)/4);
def A163920(n)
  s = 0
  (1..n).each{|i|
    s += (-1) ** (n + i) * (i + 1) * (n / i + 1) if n % i == 0
  }
  n * s / 4
end

n = 10000
(1..n).each{|i|
  j = A163920(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}