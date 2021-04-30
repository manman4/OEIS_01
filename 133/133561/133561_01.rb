require 'prime'
def A(n)
  p_ary = [0] + Prime.take(10 ** 6 + 7).to_a
  i = 0
  ary = []
  while ary.size < n
    i += 1
    m = (0..6).inject(0){|s, j| s + p_ary[i + j] ** 2}
    ary << i if m.prime?
  end
  ary
end
n = 1000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
