def A036765(n)
  a = [1, 1, 2]
  # D-finite with recurrence: 2*(n+1)*(2*n+3)*(13*n-1)*a(n) = (143*n^3 + 132*n^2 - 17*n - 18)*a(n-1) + 4*(n-1)*(26*n^2 + 11*n - 6)*a(n-2) + 16*(n-2)*(n-1)*(13*n + 12)*a(n-3).
  (3..n).each{|i|
    a << ((143 * i**3 + 132 * i**2 - 17 * i - 18) * a[-1] + 4 * (i - 1) * (26 * i**2 + 11 * i - 6) * a[-2] + 16 * (i - 2) * (i - 1) * (13 * i + 12) * a[-3]) / (2 * (i + 1) * (2 * i + 3) * (13 * i - 1))
  }
  a
end

# a(n) = 4^(n-1) - Sum_{k=0..n-2} 4^(n-2-k) * A036765(k).
def A(n)
  a_ary = A036765(n)
  a = [0]
  (1..n).each{|i|
    s = 4 ** (i - 1)
    (0..i - 2).each{|k|
      s -= 4**(i - 2 - k) * a_ary[k]
    }
    a << s
  }
  a
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
  