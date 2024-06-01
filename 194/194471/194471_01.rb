def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(n) = n! * Sum_{k=0..n} (k+1)^(n-k-1) * binomial(2*k,k)/(n-k)!.
def A(n)
  return 1 if n == 0
  m = f(n)
  (0..n).inject(0){|s, i| s + (i + 1) ** (n - i - 1) * ncr(2 * i, i) * m / f(n - i)}.to_i
end

n = 500
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
