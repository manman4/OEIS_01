require 'prime'

n = 10
p_ary = [0] + Prime.take(n)
str = ""
b=[]
(1..n).each{|i|
  str += p_ary[i].to_s(2)
  break if str.to_s.size > 1000
  print i
  print ' '
  puts str
  b<<str.to_i
}
p b