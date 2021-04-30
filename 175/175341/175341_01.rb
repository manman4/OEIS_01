def A(n)
  n2= n * n
  a = Array.new(n2 + 1, 0)
  (-n).upto(n){|i|
    (-n).upto(n){|j|
      if i.gcd(j) == 1
        k = i * i + j * j
        if k <= n2
          a[k] += 1
        end
      end
    }
  }
  b = [0]
  s = 0
  (1..n2).each{|i|
    s += a[i]
    b << s
  }
  (0..n).map{|i| b[i * i]}
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
