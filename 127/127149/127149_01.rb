require 'prime'

ary = []
cnt = 0
max = -1
(1..10000000).each{|i|
  if i.prime?
    cnt += 1
    a = i % cnt
    if max < a
      max = a
      ary << a
    end
  end
}


(1..10000).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
