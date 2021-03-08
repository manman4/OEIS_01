def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(n)
  a = bernoulli(2 * n)
  ary = []
  m = 0
  (0..2 * n).each{|i|
    j = a[i].denominator
    if j > m
      ary << j
      m = j
    end
  }
  ary
end

m = 100
# m=100でn=15まで
p ary = A(m)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
