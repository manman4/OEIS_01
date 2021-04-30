def f(n, m = 0, a = [])
  if n <= m
    @ary[a.clone.max] += 1 if n == m && a[-1] == 1
  else
    s = 1
    s = [1, a[-1] - 1].max if m > 0
    (s..n).each{|x|
      a.push x
      f(n, m + x, a)
      a.pop
    }
  end
end

n = 25
ary = [1]
(1..n).each{|i|
  @ary = Array.new(i + 1, 0)
  f(i)
  ary << @ary
}
ary.flatten!
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
