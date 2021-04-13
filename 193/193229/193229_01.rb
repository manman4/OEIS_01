def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A193229(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << f(2 * i - j) / (2 ** (i - j) * f(i - j))
    }
  }
  ary
end

n = 139
ary = A193229(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
