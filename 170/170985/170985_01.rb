def carryless(f_ary, b_ary, base)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary.map{|i| i % base}.join.to_i(base)
end

def T(n, k, base)
  carryless(n.to_s(base).split('').map(&:to_i), k.to_s(base).split('').map(&:to_i), base)
end

n = 4 ** 6 - 1
(0..n).each{|i| 
  print i
  print ' '
  puts T(i, i, 4)
}
