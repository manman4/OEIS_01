def A115245(n)
  a = [3]
  (1..n).each{|i| a << a[-1] + 3 ** 2 ** i}
  a
end
n = 12
ary = A115245(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
