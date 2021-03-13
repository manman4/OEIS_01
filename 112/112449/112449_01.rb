def A(l, m, n)
  a = Array.new(2 * m, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[1..-1].inject(:*) + a[m] ** l
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end
def A112449(n)
  A(3, 1, n)
end

n = 12
p A(1, 1, n)
p A(2, 1, n)
ary = A112449(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
