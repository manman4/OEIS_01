def A165903(n)
  a = Array.new(3, 1)
  ary = [1]
  while ary.size < n + 1
    b, c = a[1], a[2]
    i = b * b + c * c + b * c
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

n = 20
ary = A165903(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
