def B(k, n)
  ary = []
  (1..n).each{|i|
    (i..n).each{|j|
      ary << i * j
    }
  }
  ary.uniq!
  ary.size
end

def C(k, n)
  [0] + (1..n).map{|i| B(k, i)}
end

n = 1000
ary = C(2, n + 1)
(1..n).each{|i|
  j = i + 1 - ary[i + 1] + ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
