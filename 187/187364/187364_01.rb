def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A187364(n)
  (0..n).map{|i| ncr(6 * i + 1, 3 * i + 1)}
end

n = 600
ary = A187364(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
