# a(n)=3*a(n-1)-3*a(n-2)+ a(n-3)+a(n-5).
def a(n)
  ary = [0, 0, 1, 3, 6]
  (5..n).each{|i|
    ary << 3*ary[-1] - 3*ary[-2] + ary[-3] + ary[-5]
  }
  ary
end

n = 1000
ary = a(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
} 