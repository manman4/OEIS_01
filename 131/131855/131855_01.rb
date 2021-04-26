def A(n)
  s = 0
  n.to_s(2).split('').reverse.each_with_index{|e, i| s += e.to_i * 1i ** i}
  s
end

def B(n)
  s = -1
  ary = []
  while ary.size < n
    s += 1
    ary << s if A(s).imaginary == 0
  end
  ary
end

n = 10000
ary = B(n)
(1..n).each{|i| 
  print i
  print ' '
  puts ary[i - 1]
}