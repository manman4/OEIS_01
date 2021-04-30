require 'prime'

def A(n)
  ary = []
  (1..n).each{|i|
    m = 2 * i - 1
    cnt = 0
    (0..m).each{|j|
      k = j * j + j + m
      cnt += 1 if k.prime?
    }
    ary << cnt
  }
  ary
end

n = 5000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
