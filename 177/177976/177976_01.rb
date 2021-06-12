require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  s = 0
  (1..n).each{|i| 
    (1..i).each{|j|
      s += A008683(i / j) * ncr(j + k - 2, j - 1) if i % j == 0
    }
  }
  s
end

# upward
def A177976(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << A(j, i - j + 1)
    }
  }
  ary
end

n = 140
ary = A177976(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
