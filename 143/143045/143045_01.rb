# a(1) = 1; a(n) = (-1)^n * Sum_{k=1..n-1} a(k)*a(n-k).
def a(n)
  ary = [0, 1]
  (2..n).each{|i| ary << (-1)**i * (1..i - 1).inject(0){|s, k| s + ary[k] * ary[i - k]}}
  ary
end

p a(31)[1..-1]

# n = 1000
# ary = a(n)
# (1..n).each{|i|
#   j = ary[i]
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }


