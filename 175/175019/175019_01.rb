@ary = [1, 1]

def a(n)
  return 1 if n == 0 || n == 1
  i = @ary[n - @ary[n - 2]] + 2
  @ary[n] = i
  i
end

n = 10000
(0..n).each{|i|
  print i
  print  ' '
  puts a(i)
}
