def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A180875(n)
  a = [1]
  (1..n).each{|i| a << a[-1] + 1 + (0..i - 1).inject(0){|s, j| s + ncr(i, j) * a[j]}}
  a
end

n = 500
ary = A180875(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
