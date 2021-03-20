def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (1..n).inject(1){|s, i| s * i ** ((-1) ** (i % 2) * ncr(n - 1, i - 1))}
end

n = 20
(1..n).each{|i|
  j = A(i).denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
