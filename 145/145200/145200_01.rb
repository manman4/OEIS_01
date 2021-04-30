require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# x > 0
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
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

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def E0_2k(k, n)
  a = -4 * k / bernoulli(2 * k)[-1]
  [1] + (1..n).map{|i| a * sigma(2 * k - 1, i)}
end

def A000521(n)
  s3 = [0] + (1..n + 1).map{|i| sigma(3, i)}
  s5 = [0] + (1..n + 1).map{|i| sigma(5, i)}
  ary = [1]
  (0..n).each{|i| ary << (1..i + 1).inject(0){|s, j| s + (504 * s5[j] - 240 * (i - j) * s3[j]) * ary[-j]} / (i + 1)}
  ary
end

def A145200(n)
  e2 = E0_2k(1, n)
  e4 = E0_2k(2, n)
  e6 = E0_2k(3, n)
  j  = A000521(n)
  mul(mul(e2, e4, n), I(mul(e6, j, n), n), n)
end

n = 20
ary = [0] + A145200(n - 1).map{|i| i.to_i}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
