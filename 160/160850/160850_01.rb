def f(m, n, k)
  s = 0
  t, u = n / k, m / k
  (-t).upto(t){|x|
    (-u).upto(u){|y|
      if x.gcd(y) == 1
        s += (n - (k * x).abs) * (m - (k * y).abs)
      end
    }
  }
  s
end

def g(m, n)
  return 0 if m == 0 || n == 0
  (f(m, n, 1) - f(m, n, 2)) / 2
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts g(10, i)
}
