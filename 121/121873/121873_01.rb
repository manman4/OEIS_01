def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n+1) = (1/(n+1)) * Sum_{k=0..n} binomial(n+k,k) * binomial(2*n+k+2,n-k).
def A(n)
  (0..n).inject(0){|sum, k| sum + ncr(n + k, k) * ncr(2 * n + k + 2, n - k)} / (n + 1)
end

def B(n)
  A(n - 1)
end

n = 1000
(1..n).each{|i|
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
