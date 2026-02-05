def A181999(n)
  ary = [1]
  (1..n).each{|i|
    ary << ((i + 1) % 2) * ary[i / 2] + (0..i - 1).inject(0){|s, j| s + ary[j] * ary[i - 1 - j]}
  }
  ary
end

n = 1000
ary = A181999(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}