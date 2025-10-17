# a(0) = a(1) = 1; a(n) = a(n-1) - 3*a(n-2) + Sum_{k=0..n-2} a(k) * a(n-2-k). 
def A(n)
  a = [1, 1]
  (2..n).each{|i|
    a << (0..i - 2).inject(a[-1] - 3 * a[-2]){|sum, k| sum + a[k] * a[i - 2 - k]}
  }
  a
end
  
n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
