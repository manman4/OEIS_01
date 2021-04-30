# m���ȉ������o��
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

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

m = 10000
n = m / 2 + 1
ary0 =
[0, 0, 0, 0, 0, 16, 80, -544, -1856, 8080, 9856,
 -50864, -64, 152576, -130816, -214272, 245760,
 222208, 44544, -53248, -352256, 81920, 32768]
ary1 =
[1, -6, -64, 200, 1000, -3016, -3488, 24256,
 -23776, -104168, 203408, 184704, -443392,
 -14336, 151296, -145920, 263424, -317440,
 -36864, 966656, -573440, -131072]
ary1 = ary1 + [0] * (n - (ary1.size - 1))
ary = mul(ary0, I(ary1, n), n)
(1..m).each{|i|
  j = 0
  j += ary[i / 2] if i % 2 == 0
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
