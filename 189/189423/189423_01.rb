def A047974(n)
  ary = [1, 1]
  (2..n).each{|i| ary << ary[-1] + 2 * (i - 1) * ary[-2]}
  ary
end

def A(n)
  a_ary = A047974(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (i - 1) * a[j + 1]
    }
    a = b
    ary << (0..i).inject(0){|s, j| s + a_ary[j] * (-1) ** (i - j) * a[j]}
  }
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}