def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(n)
  s = 0
  (1..n).each{|i|
    s += ncr(n, i) if n % i == 0
  }
  s
end

def B(n)
  ary = [1]
  a = [0] + (1..n).map{|i| A(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 4000
ary = B(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
