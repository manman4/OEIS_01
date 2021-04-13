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

def add(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  s = [s1, s2].max
  ary = Array.new(s, 0)
  (0..s1 - 1).each{|i| ary[i] += f_ary[i]}
  (0..s2 - 1).each{|i| ary[i] += b_ary[i]}
  ary[0..m]
end

def A(n)
  return [[1]] if n == 0
  ary = [[1], [1]]
  m = 10 ** 5
  (2..n).each{|i| ary << add(ary[-1], [0] * i + ary[-2], m)}
  ary
end

n = 47
ary = A(n).flatten
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
