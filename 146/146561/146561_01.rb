def A(n)
  m = (n + 6) / 7
  ary = []
  (1..m).each{|i|
    (3..9).each{|j|
      ary << j * (10 ** (28 * i) - 1) / 29
    }
  }
  ary[0..n - 1]
end

n = 300
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
