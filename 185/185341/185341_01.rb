def u(n)
  a = Array.new(3, 1r)
  ary = [1]
  while ary.size < n
    j = a[1] + a[-1]
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

n = 300
ary = u(n)
(1..n).each{|i|
  j = ary[i - 1].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
