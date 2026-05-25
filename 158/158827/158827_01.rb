def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{0 = x_0 <= x_1 <= ... <= x_4 = n-1} Product_{k=0..3} (x_k + 1) * binomial(2*x_{k+1} - x_k + 1,x_{k+1} - x_k)/(2*x_{k+1} - x_k + 1).
# x_0 =0, x_4 = n-1に注意
def a(n)
  sum = 0
  (0..n-1).each{|x1|
    (x1..n-1).each{|x2|
      (x2..n-1).each{|x3|
        x = [0, x1, x2, x3, n-1]
        prod = 1
        (0..3).each{|k|
          prod *= (x[k] + 1) * ncr(2*x[k+1] - x[k] + 1, x[k+1] - x[k]) / (2*x[k+1] - x[k] + 1r)
        }
        sum += prod
      }
    }
  }
  sum.to_i
end

n = 50
b=[]
(1..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
  b<<j
}
p b
