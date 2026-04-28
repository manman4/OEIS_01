# a(0) = 1; a(n) = |a(n-1) - n| if n is not one of the earlier terms of the sequence, otherwise a(n) = a(n-1) + n.
def A(n)
  a = [1]
  (1..n).each{|i|
    if a.include?(i)
      a << a[-1] + i
    else
      a << (a[-1] - i).abs
    end
  }
  a
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}