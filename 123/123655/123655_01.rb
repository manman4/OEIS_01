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

def A123655(n)
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, 1
    ary = mul(ary, b_ary, n)
  }
  ary2 = Array.new(n + 1, 0)
  (0..n / 2).each{|i| ary2[2 * i] = ary[i]}
  ary4 = Array.new(n + 1, 0)
  (0..n / 4).each{|i| ary4[4 * i] = ary[i]}
  ary8 = Array.new(n + 1, 0)
  (0..n / 8).each{|i| ary8[8 * i] = ary[i]}
  ary = mul(ary, ary, n)
  ary = mul(ary, ary2, n)
  ary = mul(ary, ary4, n)
  ary = mul(ary, ary8, n)
  mul(ary, ary8, n)[0..-2]
end

n = 1000
ary = A123655(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
