def A(n)
  cnt = 0
  (1..n).each{|i|
    if n % i == 0
      j = n / i
      if i < j
        cnt += 1 if (i + j) % 2 == 0
      end
    end
  }
  cnt
end

p (1..105).map{|i| A(i)}