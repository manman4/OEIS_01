require 'prime'

def A(n)
  p_ary = [0] + Prime.take(n + 10).to_a
  ary = []
  (1..n).each{|i|
    cnt = 1
    while !(p_ary[i] * p_ary[i + 1] - cnt).prime?
      cnt += 1
    end
    ary << cnt
  }
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
