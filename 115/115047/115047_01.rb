require 'prime'
def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = [0, 0, 0, 1]
b=[]
  (4..n).each{|i|
    ary << (1..i - 3).inject(0){|s, j| s + ncr(i - 4, j - 1) * ncr(i, j + 1) * ary[j + 2] * ary[i - j] * j * (i - j - 2)} / (2 * (i - 1))
if i.prime? && ary[-1] % i > 0
p i
b <<i
end
  }
  b
end
n = 500
p ary = A(n)
=begin
(3..n).each{|i|
  j = ary[i - 3]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
=end
