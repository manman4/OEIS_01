def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  ary = [0]
  (1..n).each{|i| ary << (1..i).inject(1){|s, j| s - (k + 1) ** (j * (i - j)) * ncr(i - 1, j) * ary[-j]}}
  ary
end

n = 100
ary = A(1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}  