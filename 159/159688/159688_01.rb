def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# f(x+1) - f(x) = (aryが表す多項式) を満たすf(x)を求める
def A(ary)
  m = ary.size
  a = []
  m.downto(1){|i|
    b = ary[0] / i.to_r
    a << b
    (1..i).each{|j| ary[j - 1] -= b * ncr(i, j)}
    ary = ary[1..-1]
  }
  a
end

def B(n)
  # f(x+1) - f(x) = (1+x)^n
  (0..n).map{|i| A((0..i).map{|j| ncr(i, j)}).reject{|j| j.numerator == 0}}.flatten
end

n = 200
ary = B(n)
(0..ary.size - 1).each{|i|
  m = ary[i]
  j = m.numerator / m.abs.numerator * m.denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
