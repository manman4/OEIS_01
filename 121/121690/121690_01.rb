def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} C(k*(k+1)/2,n-k) = Sum_{k=0..n} C((n-k)*(n-k+1)/2,k).
def A(n)
  s = 0
  (0..n).each{|i|
    j = (n - i) * (n - i + 1) / 2
    break if i > j
    s += ncr(j, i)
  }
  s
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}