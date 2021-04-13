def A126116(n)
  a = Array.new(4, 1)
  ary = [1]
  while ary.size < n + 1
    a = *a[1..-1], a[0] + a[1] + a[3]
    ary << a[0]
  end
  ary
end

n = 5000
ary = A126116(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
