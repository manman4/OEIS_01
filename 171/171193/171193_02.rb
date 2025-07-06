# a(0) = 1; a(n) = 2^(n-1) * Sum_{i,j,k>=0 and i+j+k=n-1} 1/2^i * a(i) * a(j) * a(k)
def a(n, k)
  ary = [1]
  (1..n).each{|m|
    s = 0
    # i,j,k>=0 and i+j+k=m-1を列挙
    (0..m - 1).to_a.repeated_permutation(k).each{|i|
      s += 1r / (2 ** i[0]) * (1..k).inject(1){|prod, j| prod * ary[i[j - 1]]} if i.sum == m - 1
    }
    ary[m] = (2 ** (m - 1) * s).to_i
  }
  ary
end

n = 80
ary = a(n, 4)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
