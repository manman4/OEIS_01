def f(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j| ary << 2 * j - 1}
  }
  ary
end

n = 140
ary = f(n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
