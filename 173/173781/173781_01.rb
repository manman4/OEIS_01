def f(n)
  return 1 if n == 0
  (n + 1..n * 2).inject(:*)
end

def A(m, n)
  return 1 if m == 0 && n == 0
  f(m) * f(n) / (1..m + n).inject(:*)
end

def A173781(n)
  (0..n).map{|i| A(i, (i / 3.0).ceil)}
end

n = 2200
ary = A173781(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
