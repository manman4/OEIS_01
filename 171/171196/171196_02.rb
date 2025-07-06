# a(0) = 1; a(n) = 2^(n-1) * Sum_{x_1, x_2, ..., x_7>=0 and x_1+x_2+...+x_7=n-1} (1/2)^x_1 * Product_{k=1..7} a(x_k).
def a(n, k)
  ary = [1]
  (1..n).each{|m|
    s = 0
    # x_1, x_2, ..., x_7>=0 and x_1+x_2+...+x_7=m-1を列挙
    (0..m - 1).to_a.repeated_permutation(k).each{|i|
      s += 1r / (2 ** i[0]) * (1..k).inject(1){|prod, j| prod * ary[i[j - 1]]} if i.sum == m - 1
    }
    ary[m] = (2 ** (m - 1) * s).to_i
  }
  ary
end

n = 13
p ary = a(n, 7)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
