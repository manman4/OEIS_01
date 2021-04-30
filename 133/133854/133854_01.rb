def A(k, l, n)
  a = Array.new(k * 2 - 1, 1)
  ary = [1]
  while ary.size < n
    break if (a[1] * a[-1] + (a[k - 1] + a[k]) * l) % a[0] > 0
    a = *a[1..-1], (a[1] * a[-1] + (a[k - 1] + a[k]) * l) / a[0]
    ary << a[0]
  end
  ary
end

k = 7
n = 6000
ary = A(k, 1, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
