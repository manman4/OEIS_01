def A142071(n)
  a = [0, 1]
  b = [0, 1]
  i = 2
  while a.size < n
    b << 0
    i.downto(1){|j|
      # b[j]��b[j - 1] * (j - 1) + b[j] * j�ɂ���
      b[j] += (b[j - 1] + b[j]) * (j - 1)
    }
    a += b
    i += 1
  end
  a[0..n - 1]
end

# このコードはこの数列の定義が変わる前に書いた
n = 10000
ary = A142071(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
