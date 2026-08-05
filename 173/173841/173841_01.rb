def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n,k) = Sum_{j=0..m} (-2)^j*binomial(m,j)*(n-j)! where m = max(0, floor((n-k+1)/2)).
def a(n, k)
  m = [0, (n - k + 1) / 2].max
  (0..m).inject(0){|s, j| s + (-2)**j * ncr(m, j) * f(n - j)}
end

n = 1000
(0..n).each{|i|
  j = a(i, 1) 
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
