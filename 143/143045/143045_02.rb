def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(2n)
def a_even(n)
  raise ArgumentError, "n must be > 0" if n <= 0
  s = 0
  (0..n-1).each{|k|
    s += (-2)**k *
         ncr(2*n + k - 1, k) *
         ncr(2*n - k - 2, n - k - 1)
  }
  s / n
end

# a(2n+1)
def a_odd(n)
  raise ArgumentError, "n must be > 0" if n <= 0
  s = 0
  (0..n-1).each{|k|
    s += (-2)**k *
         ncr(2*n + k + 1, k) *
         ncr(2*n - k - 2, n - k - 1)
  }
  -2 * s / n
end

# a(m)（偶奇で自動分岐）
def a(m)
  return 1 if m == 1
  m.even? ? a_even(m / 2) : a_odd((m - 1) / 2)
end

n = 31
(1..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print " "
  puts j
}
