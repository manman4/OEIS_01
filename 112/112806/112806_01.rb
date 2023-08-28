def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..floor(n/2)} binomial(n-k-1,k) * binomial(2*n-k+1,n-2*k)/(2*n-k+1). 
def A112806(n)
  return 1 if n == 0
  (0..n / 2).inject(0){|s, i| s + ncr(n - i - 1, i) * ncr(2 * n - i + 1, n - 2 * i) / (2 * n - i + 1r)}.to_i
end

n = 1000
(0..n).each{|i|
  j = A112806(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
