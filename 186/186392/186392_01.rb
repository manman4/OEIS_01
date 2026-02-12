def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} C(n,k)^2*C(6k,3k).
def A186392(n)
  (0..n).inject(0){|s, k| s + ncr(n, k)**2 * ncr(6 * k, 3 * k)}
end

n = 1000
(0..n).each{|i|
  j = A186392(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}