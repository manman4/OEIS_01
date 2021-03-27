def A(m, n)
  m + n - m.gcd(n)
end

ary = []
(1..140).each{|i|
  (1..i).each{|j|
    ary << A(i, j)
  }
}
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
