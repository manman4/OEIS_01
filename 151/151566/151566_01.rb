def A006046(n)
  ary = [0, 1]
  (2..n).each{|i|
    ary << 2 * ary[i / 2] + ary[i / 2 + i % 2]
  }
  ary
end

def A000120(n)
  ary = [0]
  (1..n).each{|i|
    ary << ary[i / 2] + i % 2
  }
  ary
end

def A001316(n)
  a = A000120(n)
  (0..n).map{|i| 2 ** a[i]}
end

def A151566(n)
  a = A006046(n)
  b = A001316(n)
  (0..n).map{|i| 2 * a[i / 2] + (i % 2) * b[i / 2]}
end

m = 10100
n = 10000
ary = A151566(m)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
