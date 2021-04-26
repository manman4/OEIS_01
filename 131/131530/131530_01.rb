require 'prime'

def A(n)
  i = 2
  ary = []
  while ary.size < n
    j = i * i - i - 1
    if j.prime?
      ary << i if (j + 2).prime?
    end
    i += 1
  end
  ary
end

n = 5000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
