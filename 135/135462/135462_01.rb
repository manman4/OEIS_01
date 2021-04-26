require 'prime'

def A(n)
  ary = [0, 0, 0]
  cnt = 1
  (3..n).each{|i|
    sq = Math.sqrt(i).to_i
    if i.prime? && i % 6 == 1
      cnt += 2
    elsif sq ** 2 == i
      cnt += 1 if sq.prime? && sq % 3 == 2
    end
    ary << cnt
  }
  ary[0..n]
end

n = 100
p ary = A(n).map{|i| i * 6}
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
