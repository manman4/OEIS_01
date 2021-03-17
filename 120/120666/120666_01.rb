def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << f(i * j) / f(i) ** j
    }
  }
  ary
end

n = 26
ary = A(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
