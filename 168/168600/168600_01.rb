def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(0) = 1; a(n) = 2^(n-1) * (n-1)! * Sum_{i, j, k>=0 and i+j+k=n-1} (n-i)/2^i * a(i) * a(j) * a(k)/(i! * j! * k!). 
def a(n, k)
  ary = [1]
  (1..n).each{|m|
    s = 0
    # i,j,k>=0 and i+j+k=m-1を列挙
    (0..m - 1).to_a.repeated_permutation(k).each{|i|
      s += (m - i[0]).to_r / (2 ** i[0]) * (1..k).inject(1r){|prod, j| prod * ary[i[j - 1]] / f(i[j - 1])} if i.sum == m - 1
    }
    ary[m] = (2 ** (m - 1) * f(m - 1) * s).to_i
  }
  ary
end

n = 76
ary = a(n, 3)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}