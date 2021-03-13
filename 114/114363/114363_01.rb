def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(n)
  a = bernoulli(4 * n)
  ary = []
  (0..n).each{|i|
    ary << -2 * a[4 * i] / (a[2 * i] ** 2 * ncr(4 * i, 2 * i))
  }
  ary
end

n = 200
ary = A(n)
(0..n).each{|i|
  j = ary[i].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
