def T(n, k)
  (k..n).select{|i| n % i == 0}.size
end

def A135539(n)
  ary = []
  (1..n).each{|i|
    ary << (1..i).map{|j| T(i, j)}
  }
  ary.flatten
end

n = 140
ary = A135539(n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}