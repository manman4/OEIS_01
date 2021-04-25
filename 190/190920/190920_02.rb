def mul(f_ary, b_ary)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary
end

def power(ary, n)
  return [1] if n == 0
  k = power(ary, n >> 1)
  k = mul(k, k)
  return k if n & 1 == 0
  return mul(k, ary)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(i, n)
  ary = [0] + (1..i).map{|j| (-1) ** (i - j) * ncr(i - 1, i - j) / f(j).to_r}
  ary = power(ary, n)
  (0..ary.size - 1).inject(0){|s, i| s + f(i) * ary[i]}.to_i / f(n)
end

n = 500
(1..n).each{|i|
  j = A(i, 5)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
