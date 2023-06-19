require 'prime'

# x^2 + y^3 = n 
def A(n)
  s = 0
  (1..n).each{|i|
    break if i * i > n
    (1..n).each{|j|
      k = i * i + j * j * j
      break if k > n
      s += 1 if k == n
    }
  }
  s
end

n = 1000000
ary = []
(1..n).each{|i|
  if i.prime?
    j = A(i)
    if j > 1 
      ary << i
    end
  end
}

(1..500).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}

