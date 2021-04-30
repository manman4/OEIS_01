require 'prime'

def A(n)
  a, b, c = 3, 0, 2
  cnt = 0
  ary = []
  while ary.size < n
    a, b, c = b, c, a + b
    cnt += 1
    ary << cnt if cnt.prime? && a % (cnt * cnt) == 0
  end
  ary
end

n = 2
p ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
