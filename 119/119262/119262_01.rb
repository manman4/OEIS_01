def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = [0, 1]
  (2..n).each{|i| 
    ary << (1..i / 2).inject(0){|s, j| s + ncr(i - j - 1, i - 2 * j) * ary[j]}
  }
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}