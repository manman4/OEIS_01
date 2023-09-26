def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# T(n,k)=n!/(n - k)! + n!/k! - n!
def T(n, k)
  f(n) / f(n - k) + f(n) / f(k) - f(n)
end

def A177974(n)
  (0..n).map{|i| (0..i).map{|j| T(i, j)}}.flatten
end

n = 139
ary = A177974(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
