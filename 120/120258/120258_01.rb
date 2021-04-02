def f(k, n)
  return 1 if k == 0
  (n + 1..n + k).inject(:*)
end

def T(n, k)
  (0..k - 1).inject(1){|s, i| s * f(n, i + n) / f(n, i).to_r}.to_i
end

def A(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << T(i - j, j)
    }
  }
  ary
end

n = 100
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
