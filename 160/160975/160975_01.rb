# m次以下を取り出す
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

def B(k, n)
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(k * i + 1, 0)
    b_ary[0], b_ary[i], b_ary[k * i] = 1, -1, 1
    ary = mul(ary, b_ary, n)
  }
  ary
end

def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def C(n)
  ary = [1] * (n + 1)
  A(ary, ary, n)
end

def D(k, n)
  mul(B(k, n), C(n), n)
end

n = 1100
m = 1000
ary = D(5, n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
