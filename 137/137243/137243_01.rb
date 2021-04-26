def f(m, n)
  s = 0
  (-m).upto(m){|x|
    (-n).upto(n){|y|
      if x.gcd(y) == 1
        s += 1
      end
    }
  }
  s
end

n = 1000
(1..n).each{|i|
  print i
  print ' '
  puts f(i, i)
}
