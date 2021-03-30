def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  ncr(n - 1, k - 1) * ncr(n, k + 1) * ncr(n + 1, k)
end

n = 141
cnt = 2
(2..n).each{|i|
  (1..i - 1).each{|j|
    m = A(i, j)
    break if m.to_s.size > 1000
    print cnt
    print ' '
    puts m
    cnt += 1
  }
}