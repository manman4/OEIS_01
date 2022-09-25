def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  m = f(n)
  (0..n).inject(0){|s, i| s + (-1) ** (n - i) * (i + 1) ** (i - 1) * ncr(n, i) * m / f(i)}
end

def A108919(n)
  A(n - 1)
end

n = 500
(1..n).each{|i|
  j = A108919(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

