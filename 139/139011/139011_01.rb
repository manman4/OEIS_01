# n-1ŒÂ
def A(k, n)
  ary = [[0]]
  a = [0]
  l2_ary = []
  (1..n).each{|i|
    m = 0
    m += k ** (i / 2) if i % 2 == 1
    b = [m]
    (1..i).each{|j| b[j] = (k - 1) * a[j - 1] + b[j - 1]}
    a = b
    ary << a
    l2_ary << a[-2]
  }
  l2_ary
end

n = 3000
ary = A(-1, n + 1)
(0..n).each{|i|
  j = (-1) ** (i % 2) * ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
