def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  return -1 if n == 0
  (0..n + 1).inject(0){|s, i| s + (-1) ** (i % 2) * ncr(n + 1, i) * f(2 * n - i)} / f(n - 1)
end

n = 400
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
