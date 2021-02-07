def A105236(n)
  a = Array.new(5, 1)
  ary = [1]
  while ary.size < n + 1
    j = a[1] * a[-1] + 2 * a[2] * a[-2]
    break if j % a[0] > 0
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

n = 160
ary = A105236(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
