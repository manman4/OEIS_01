require 'prime'

def A178316(n)
  ary = []
  a = [0, 1, 2, 3, 3, 5, 9, 3, 8, 6]
  Prime.each(10 ** 8){|i|
    b = i.to_s.split('').map(&:to_i)
    if !b.include?(3) && !b.include?(4) && !b.include?(7)
      c = b.reverse.map{|i| a[i]}.join.to_i
      ary << i if c.prime?
    end
  }
  ary[0..n - 1]
end

n = 10000
ary = A178316(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
