def A(n)
  a = [1]
  while a.size < n
    b = []
    a.each{|i|
      if i == 1
        b << 1
        b << 1
        b << 2
        b << 1
      else
        b << 2
      end
    }
    a = b
  end
  a[0..n - 1]
end

m = 10010
ary = A(m)
(1..10000).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
