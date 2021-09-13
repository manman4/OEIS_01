require 'prime'

n = 10000

s = 0
ary = Prime.take(n)
(1..n).each{|i|
  s += ary[i - 1].to_s(2).to_i
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
