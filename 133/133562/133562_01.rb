require 'prime'

n = 34
p_ary = [0] + Prime.take(n + 7).to_a
(1..n).each{|i|
  print i
  print ' '
  puts (0..6).inject(0){|s, j| s + p_ary[i + j] ** 2}
}
