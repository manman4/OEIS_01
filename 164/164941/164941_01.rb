def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A164941(n)
  s = 0
  (1..n).each{|i|
    s += phi(n / i) ** (i - 1) if n % i == 0
  }
  s
end

n = 5000
(1..n).each{|i|
  j = A164941(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
