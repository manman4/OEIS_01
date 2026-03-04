def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = (1/n) * Sum_{k=0..floor((n-1)/2)} 2^(n-1-2*k) * binomial(2*n-2-k,n-1) * binomial(n-1-k,k).
def a(n)
  ary = [1]
  (1..n).each{|i| ary << (0..(i - 1) / 2).inject(0){|s, k| s + 2 ** (i - 1 - 2 * k) * ncr(2 * i - 2 - k, i - 1) * ncr(i - 1 - k, k)} / i}
  ary
end

n = 1000
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}