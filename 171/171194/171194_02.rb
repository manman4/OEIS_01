# a(0) = 1; a(n) = 2^(n-1) * Sum_{i, j, k, l, m>=0 and i+j+k+l+m=n-1} (1/2)^i * a(i) * a(j) * a(k) * a(l) * a(m).
def a(n, k)
  ary = [1]
  (1..n).each{|m|
    s = 0
    # i,j,k,l,m>=0 and i+j+k+l+m=m-1を列挙
    (0..m - 1).to_a.repeated_permutation(k).each{|i|
      s += 1r / (2 ** i[0]) * (1..k).inject(1){|prod, j| prod * ary[i[j - 1]]} if i.sum == m - 1
    }
    ary[m] = (2 ** (m - 1) * s).to_i
  }
  ary
end

n = 30
p ary = a(n, 5)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
