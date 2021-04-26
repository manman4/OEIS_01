def f(m, n)
  return 1 if n <= m
  a, b = @ary[n - 1], @ary[n - m]
  if n > [a, b].max
    i = @ary[n - a] + @ary[n - b]
    @ary[n] = i
    return i
  end
end

def A(m, n)
  ary = []
  (1..n).each{|i|
    if f(m, i) != nil
      ary << f(m, i)
    else
      break
    end
  }
  ary
end

def A134680(n)
  ary = [6]
  (2..n).each{|i|
    if i == 2 || i == 4
      ary << 0
    else
      @ary = [0] + [1] * i
      ary << A(i, 10 ** 6).size
    end
  }
  ary
end

n = 10000
ary = A134680(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
