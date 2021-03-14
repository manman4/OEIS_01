def A(n)
  cnt = 0
  (1..n - 1).each{|i|
    j = i * i + (n - i) * (n - i)
    if Math.sqrt(j).to_i ** 2 == j
      cnt += 1
    end
  }
  cnt
end

def B(n)
  s = 1
  ary = []
  while ary.size < n
    if A(s) > 0
      ary << s
    end
    s += 1
  end
  ary
end

n = 1000
ary = B(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
