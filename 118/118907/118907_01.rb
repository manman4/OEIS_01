# a(0) = 1; a(n) = |a(n-1) - n| if n is has already appeared in the sequence, otherwise a(n) = a(n-1) + n.
def A(n)
  a = [1]
  (1..n).each{|i|
    if a.include?(i)
      a << (a[-1] - i).abs
    else
      a << a[-1] + i
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