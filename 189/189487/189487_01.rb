def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = n! * Sum_{m=1..n} (1/m!) * Sum_{k=m..n} k^(n-k)*binomial(m,k-m)/(n-k)!, n>0, a(0)=1.
def A(n)
  return 1 if n == 0
  m = f(n)
  s = 0
  (1..n).each{|i|
    t = 0
    (i..n).each{|j|
      t += j ** (n - j) * ncr(i, j - i) / f(n - j).to_r if i >= j - i
    }
    s += t * m / f(i)
  }
  s.to_i
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}