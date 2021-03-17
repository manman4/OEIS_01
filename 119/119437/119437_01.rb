def f(n, k)
  s = 0
  m = n / k
  (-m).upto(m){|x|
    (-m).upto(m){|y|
      if x.gcd(y) == 1
        s += (n - (k * x).abs) * (n - (k * y).abs)
      end
    }
  }
  s
end

def g(k, n)
  return 0 if n == 0
  (f(n, k + 1) - 2 * f(n, k) + f(n, k - 1)) / 2
end

n = 141
ary = []
(2..n).each{|i|
  (2..i).each{|j| ary << g(j, i)}
}
(2..ary.size + 1).each{|i|
  print i
  print ' '
  puts ary[i - 2]
}
