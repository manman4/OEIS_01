def A(n)
  ary = []
  (1..n).each{|a|
    (1..a).each{|b|
      (1..b).each{|c|
        (1..c).each{|d|
          ary << a * b * c * d
        }
      }
    }
  }
  ary.uniq.size
end

n = 200
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
