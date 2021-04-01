require 'prime'

p_ary = [0] + Prime.take(10 ** 8 + 100).to_a

n = 10000
(1..n).each{|i|
  j = p_ary[i] ** 2 - p_ary[i * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
