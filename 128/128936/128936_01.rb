def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  n.to_s.split('').inject(0){|s, i| s + i.to_i}
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts ncr(i, A(i))
}
