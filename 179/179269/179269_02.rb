def A(n)
  return 1 if n == 0
  s = 1
  a = (1..n - 1).map{|i| [0, i]}
  while a.size > 0
    b = []
    a.each{|i|
      l, m = i[-2], i[-1]
      (m - l + 1..n - l - m).each{|j|
        k = i.clone
        k << m + j
        sum = k.inject(:+)
        if sum < n
          b << k
        elsif sum == n
          s += 1
        end
      }
    }
    a = b
  end
  s
end
(0..200).each{|i|
  print i
  print ' '
  puts A(i)
}
