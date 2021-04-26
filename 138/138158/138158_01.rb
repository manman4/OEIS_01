# méüà»â∫ÇéÊÇËèoÇ∑
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

def plus(a, b)
  s1, s2 = a.size, b.size
  n = [s1, s2].max
  ary = a + [0] * (n - s1)
  (0..s2 - 1).each{|i| ary[i] += b[i]}
  ary
end

def A(n)
  ary = [[1]]
  s = 0
  (1..n).each{|i|
    a = [0]
    (0..i - 1).each{|j|
      a = plus(a, [0] * (i - 1 - j) + mul(ary[j], ary[i - 1 - j], 2 * s))
    }
    a = [0] + a
    s = a.size - 1
    ary << a
  }
  ary.flatten
end

n = 38
ary = A(n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
