def A010816(n)
  ary = Array.new(n + 1, 0)
  # ���R�r�̌����̕K�v�ȂƂ��낾�����o��
  i = 0
  j, k = 2 * i + 1, i * (i + 1) / 2
  while k <= n
    i & 1 == 1? ary[k] = -j : ary[k] = j
    i += 1
    j, k = 2 * i + 1, i * (i + 1) / 2
  end
  ary
end

def A133089(n)
  ary = A010816(n)
  (0..n).map{|i| (-1) ** i * ary[i]}
end

def A133079(n)
  ary = A133089(3 * n)
  (0..n).map{|i| ary[3 * i]}
end

n = 10000
ary = A133079(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
