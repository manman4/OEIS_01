def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# n > 1
def A(n)
  ary = [1, 1, 1]
  (2..n).each{|i|
    ary << 1
    (1..i).each{|j|
      ary << ncr(2 * i, 2 * j) - 2 * i * ncr(i - 2, j - 1)
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
