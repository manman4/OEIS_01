def A(n)
  ns = n.to_s(2).size
  s = 1
  while (s ** n).to_s(2)[0..ns - 1] != n.to_s(2)
    s += 1
  end
  s
end

n = 5000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
