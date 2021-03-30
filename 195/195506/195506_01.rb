def A(n)
  ary = [1]
  s = 1
  (2..n).each{|i|
    s += 1r / i
    ary << ary[-1] + s / (i * i)
  }
  ary
end

n = 800
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
