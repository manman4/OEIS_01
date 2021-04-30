def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A145855(n)
  s = 0
  (1..n).each{|i|
    s += (-1) ** ((n + i) % 2) * phi(n / i) * ncr(2 * i, i) if n % i == 0
  }
  s / (2 * n)
end

n = 2000
(1..n).each{|i|
  j = A145855(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
