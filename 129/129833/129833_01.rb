def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = []
  (0..n).each{|i|
    ary << (0..i).inject(0){|s, j| s + ncr(i + 1, j + 1) * ncr(i, j) * f(j)}
  }
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
