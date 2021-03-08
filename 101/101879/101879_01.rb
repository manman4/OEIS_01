def A(k, l, n)
  a = Array.new(k * 2, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[1] * a[-1] + a[k] * l
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A101879(n)
  A(1, 1, n)
end

n = 2000
ary = A101879(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
