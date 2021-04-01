def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

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

n = 10000
ary1 = A(n)
ary2 = Array.new(n + 1, 0)
(0..n / 2).each{|i| ary2[i * 2] = ary1[i]}
ary22 = mul(ary2, ary2, n)
ary = [-1] + mul(ary1, ary22, n)[0..-2]
(0..n).each{|i|
  print i
  print ' '
  puts -ary[i]
}
