def T(n)
  cnt = 0
  (2..n).each{|i|
    (i..n / i).each{|j|
      if n % (i * j) == 0
        cnt += 1 if n / (i * j) >= j
      end
    }
  }
  cnt
end

(1..8192).each{|i| 
  print i
  print ' '
  puts T(i)
}
