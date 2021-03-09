def A(n)
  ary = [1]
  s_ary = [0]
  (1..n).each{|i|
    s_ary << s_ary[-1] + 2 * i - 1
  }
  m = s_ary[-1]
  a = Array.new(m + 1){0}
  a[0] = 1
  (1..n).each{|i|
    b = a.clone
    (0..[s_ary[i - 1], m - (2 * i - 1)].min).each{|j| b[j + 2 * i - 1] += a[j]}
    a = b
    s_ary[i] % 2 == 0 ? ary << a[s_ary[i] / 2] : ary << 0
  }
  ary
end

n = 200
ary = A(4 * n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[4 * i] / 2
}
