def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# For n>0,
# a(2*n)   =  (1/n) * Sum_{k=0..n-1} (-2)^k * binomial(2*n+k-1,k) * binomial(2*n-k-2,n-k-1),
# a(2*n+1) = -(2/n) * Sum_{k=0..n-1} (-2)^k * binomial(2*n+k+1,k) * binomial(2*n-k-2,n-k-1).

# a(2n)
def a_even(n)
  raise ArgumentError, "n must be > 0" if n <= 0
  (0..n-1).inject(0){|s, k|
    s + (-2)**k *
         ncr(2*n + k - 1, k) *
         ncr(2*n - k - 2, n - k - 1)
  } / n
end

# a(2n+1)
def a_odd(n)
  raise ArgumentError, "n must be > 0" if n <= 0
  (0..n-1).inject(0){|s, k|
    s + (-2)**k *
         ncr(2*n + k + 1, k) *
         ncr(2*n - k - 2, n - k - 1)
  } * (-2) / n
end

# a(m)（偶奇で自動分岐）
def a(m)
  return 1 if m == 1
  m.even? ? a_even(m / 2) : a_odd((m - 1) / 2)
end

n = 31
p (1..n).map{|i| a(i)}
(1..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print " "
  puts j
}
