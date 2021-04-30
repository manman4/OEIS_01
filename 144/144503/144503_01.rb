def A144503(n)
  ary = []
  a = [1]
  (n + 1).times{|i|
    (1..i).each{|j|
      a[j] *= i - j
      a[j] += a[j - 1]
    }
    ary << a.inject(:+)
    a << 0
  }
  ary
end
p A144503(20) 

=begin
n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
=end
