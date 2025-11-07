def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n,k) = Sum_{j=k..floor(n/2)} (-1)^(j-k) * (j+1) * binomial(j,k) * binomial(2*n-3*j+1,n-2*j)/(2*n-3*j+1).
def a(n, k)
  (k..n / 2).inject(0){|s, j| s + (-1)**(j - k) * (j + 1) * ncr(j, k) * ncr(2 * n - 3 * j + 1, n - 2 * j) / (2 * n - 3 * j + 1)}.to_i
end

def A114486(n)
  (0..n).map{|i| (0..i / 2).map{|k| a(i, k)}}.flatten
end

n = 200
ary = A114486(n)
(0..ary.size - 1).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
