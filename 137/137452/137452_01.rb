def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << 0
    (0..i - 1).each{|j|
      ary << (-1) ** (i + j + 1) * ncr(i - 1, j) * i ** (i - 1 - j)
    }
  }
  ary
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
