def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A003319(n)
  r, c = [0], [1] + [0] * n
  (1..n).each{|i|
    i.downto(0){|j|
      c[j] = c[j - 1] * j
    }
    c[0] = -(1..i).inject(0){|s, j| s + c[j]}
    r << -c[0]
  }
  r
end

n = 500
ary = A003319(n + 1)
(1..n).each{|i|
  j = f(i - 1) * ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
