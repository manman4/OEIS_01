def A163626(n)
  a = [1]
  i = 1
  ary = [1]
  while ary.size < n + 1
    a << 0
    b = [1] + (1..i).map{|j| (j + 1) * a[j] - j * a[j - 1]}
    a = b
    i += 1
    ary += a
  end
  ary[0..n]
end

n = 10000
ary = A163626(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
