# a(n) = 2*(n-1) + a(n-2) + 2*a(n-3) - a(n-4).
def A(n)
  a, b, c, d = 1, 1, 2, 7
  ary = [1]
  while ary.size <= n
    a, b, c, d = b, c, d, 2 * d + c + 2 * b - a
    ary << a
  end
  ary
end

n = 5000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.size > 1000
  print i
  print ' '
  puts j
}