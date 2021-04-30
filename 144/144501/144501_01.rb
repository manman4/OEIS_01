def A(k, n)
  ary = Array.new(n + k + 1, 1) + [0]
  k.times{|i| (0..n + k - i - 1).each{|j| ary[j] = j * ary[j - 1] + ary[j + 1]}}
  ary[0..n]
end

n = 300
(0..n).each{|i|
  j = A(i, i)[-1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
