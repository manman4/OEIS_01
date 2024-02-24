def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n, k, q)
  s = 0
  (0..n / q - k).each{|j| s += (-1) ** j * f(n - (q - 1) * (j + k)) / f(j)}
  s / f(k)
end

# 0 <= k <= floor(n/2)
def A177248(n)
  (0..n).map{|i| (0..i / 2).map{|j| A(i, j, 2)}}.flatten
end

n = 200
ary = A177248(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}