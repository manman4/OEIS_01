def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 8 * n + 7
  s *= f(6 * n + 1)
  s /= f(3 * n)
  s /= f(n)
  s /= (f(n + 1)) ** 2
  s
end

def A001421(n)
  (0..n).map{|i| f(6 * i) / (f(3 * i) * f(i) ** 3)}
end

def B(n)
  (0..n).map{|i| A(i) / 7}
end

def C(k, n)
  a = A001421(n + k)
  b = B(n + k)
  ary = [1, 84]
  k.times{|i|
    c0 = (b[1] - a[1]).to_r
    c = (1..n + k - i).map{|j| (b[j] - a[j]) / c0}
    a, b = b, c
    ary << ary[-1] * c0
  }
  (0..n).each{|i| a[i] = a[i].to_i if a[i].denominator == 1}
  ary[0..n]
end

n = 20
ary = C(n, n)
(0..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
