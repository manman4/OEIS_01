def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n, k)=sum(j=0, k, binomial(k+j, 2*j)*(n-k)^j*binomial(2*j,j)/(j+1)).
def T(n, k)
  (0..k).inject(0){|s, i| s + ncr(k + i, 2 * i) * (n - k) ** i * ncr(2 * i, i) / (i + 1)}
end

def A(n)
  (0..n).map{|i| (0..i).map{|j| T(i, j)}}.flatten
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
