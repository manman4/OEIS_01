def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
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

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A(n)
  ary = [0] + (1..n).map{|i| 1r / f(i)}
  ary = power(ary, 3, 3 * n)
  (3..3 * n).inject(0){|s, i| s + f(i) * ary[i]}
end

n = 100
(0..n).each{|i|
  j = A(i).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
