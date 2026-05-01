# 2*n * (2*n-1) * (7*n-9) * a(n) = (329*n^3 - 752*n^2 + 491*n - 84) * a(n-1) + 6 * (n-1) * (2*n-3) * (7*n-2) * a(n-2) for n > 1.
def a(n)
  ary = [1, 4]
  (2..n).each{|i|
    ary << ((329*i**3 - 752*i**2 + 491*i - 84) * ary[-1] + 6 * (i-1) * (2*i-3) * (7*i-2) * ary[-2]) / (2*i * (2*i-1) * (7*i-9))
  }
  ary
end

n = 1000
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
