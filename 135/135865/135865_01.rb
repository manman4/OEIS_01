# a(0) = a(1) = 1; a(n) = (1/2) * (4^(n-1)*a(n-1) - Sum_{k=2..n-1} a(k) * a(n+1-k)). 
def A(n)
  ary = [1, 1]
  (2..n).each{|i|
    ary << (2..i - 1).inject((4 ** (i - 1)) * ary[-1]){|s, k| s - ary[k] * ary[i + 1 - k]} / 2.to_r
  }
  ary
end

n = 100
ary = A(n)
(0..n).each{|i| 
  break if ary[i].denominator != 1
  j = ary[i].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}