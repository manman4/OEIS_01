def a(n)
  ary = [0] * n
  (1..n).to_a.each_permutation{|i|
    s = 0
    (1..n - 1).to_a.each{|j|
      s += j * i[j - 1]
    }
    ary[s % n] += 1
  }
  ary
end

# a(1)..a(n)をflattenした配列を返す
def b(n)
  (1..n).map{|i| a(i)}.flatten
end

n = 13
ary = b(n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}