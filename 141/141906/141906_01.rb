def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def T(n)
  (0..n).map{|i| f(i * n) / f(i) ** n}
end

def A(n)
  (0..n).map{|i| T(i)}.flatten
end

n = 26
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
