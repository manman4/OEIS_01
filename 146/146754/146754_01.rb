# m”{‚µ‚½‚ç
def A(m, n)
  str = n.to_s
  (m * n).to_s == str[-1] + str[0..-2]
end

m = 5
n = 55
(1..n).each{|i|
  (1..9).each{|j|
    p [i, j] if A(m, j * (10 ** (6 * i) - 1) / 7)
  }
}
(1..n).each{|i|
  (1..9).each{|j|
    p [i, j] if A(m, j * (10 ** (42 * i) - 1) / 49)
  }
}
