def A(n)
  ary = []
  (1..n).each{|i|
    s = i ** (i - 1)
    (1..i - 1).each{|j|
      s -= j ** (i - 1)
    }
    ary << s
  }
  ary
end

n = 400
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
