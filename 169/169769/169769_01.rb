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
n2 = m / 2 + 1
n4 = m / 4 + 1
p_ary =
[1, -6, -64, 200, 1000, -3016, -3488, 24256,
 -23776, -104168, 203408, 184704, -443392,
 -14336, 151296, -145920, 263424, -317440,
 -36864, 966656, -573440, -131072]

ary00 =
[0, 0, 0, 0, 0, 16, 80, -544, -1856, 8080, 9856,
 -50864, -64, 152576, -130816, -214272, 245760,
 222208, 44544, -53248, -352256, 81920, 32768]
ary01 = p_ary + [0] * (n2 - (p_ary.size - 1))
ary02 = I(ary01, n2)
c_ary = mul(ary00, ary02, n2)

ary10 =
[0, 0, 4, 0, -124, 212, 1248, -5120, -4928,
 41280, 22224, -140608, -151680, 189952,
 413056, -50432, -121344, -141312, -337920,
 49152, 163840, 65536]
ary11 = p_ary + [0] * (n4 - (p_ary.size - 1))
ary12 = I(ary11, n4)
a_ary = mul(ary10, ary12, n4)

ary20 =
[0, 0, 0, 0, 0, 4, 0, -16, 48, -152, 64, 576, -1432,
 1584, -672, -5816, 15792, -2928,
 -6864, 14832, -65232, -23584, 79776,
 70944, 95936, 71552, -286848, -395648,
 -31104, -22016, 362752, 538880, 156160,
 107520, 88576, -96256, -225280, -249856,
 -454656, -311296, 196608, 294912, 65536]
ary21 = Array.new(n2 + 1, 0)
(0..n2 / 2).each{|i| ary21[2 * i] = ary02[i]}
b_ary = mul(ary20, ary21, n2)

ary30 =
[0, 0, 0, 16, 28, -364, -336, 696, -2000,
 26448, 45808, -211552, -223232, 585600,
 435072, -589312, -228864, -45056, -88064,
 360448, -131072, 65536]
e_ary = mul(ary30, ary12, n4)

(4..m).each{|i|
  j = 0
  j += c_ary[i / 2] + b_ary[i / 2] if i % 2 == 0
  j += a_ary[(i - 2) / 4] + e_ary[(i - 2) / 4] if i % 4 == 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j / 4
}