n = 150
p_ary = [0] + (1..n).map{|i| i}
ary = Array.new(30000 + 1, 0)
a = [0]
(1..n).each{|i|
  p = p_ary[i]
  b = []
  a.each{|j|
    k = j + p
    if ary[k] == 0
      ary[k] = i
    end
    b << k
    k = j - p
    if ary[k] == 0
      ary[k] = i
    end
    b << k
  }
  a = b.uniq
}

# p ary[0..10000]
(0..10000).each{|i|
  print i
  print ' '
  puts ary[i]
}
