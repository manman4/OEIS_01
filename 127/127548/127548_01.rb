def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A127548(n)
  return 1 if n == 0
  (1..n).inject(0){|s, i|
    s + (-1) ** ((n - i) % 2) * f(i) * ncr(i + n - 1, 2 * i - 1)
  }
end

n = 500
(0..n).each{|i|
  j = A127548(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
