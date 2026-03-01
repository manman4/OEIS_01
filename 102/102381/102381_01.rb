# A086595(n=1..36)のデータはb086595.txtに保存されている。
# 形式は
# n a(n)
# a(n) = n*A086595(n).

n = 36
a086595 = File.read("b086595.txt").split("\n").map{|l| l.split.map(&:to_i)}.to_h
(1..n).each{|i|
  print i
  print ' '
  puts i * a086595[i]
}

