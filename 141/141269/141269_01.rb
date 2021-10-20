require 'prime'

def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    i.prime_division.each{|j|
      ary << j[0]
      ary << j[1] if j[1] > 1
    }
  }
  # 念のため
  if ary.size >= 2 * n
    (1..n).each{|i|
      print i
      print ' '
      puts ary[2 * i - 2] + ary[2 * i - 1]
    }
  end
end

A(10000)
