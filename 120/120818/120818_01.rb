def A(n)
  s1, s2 = 2, 8
  n.times{|i|
    m = 10 ** (i + 1)
    (0..9).each{|j|
      k1, k2 = j * m + s1, (9 - j) * m + s2
      if (k1 ** 5 - k1) % (m * 10) == 0 && (k2 ** 5 - k2) % (m * 10) == 0
        s1, s2 = k1, k2
        break
      end
    }
  }
  s2.to_s.reverse
end

n = 9999
str = A(n)
(0..n).each{|i|
  print i
  print ' '
  # nil.to_i‚Í0
  puts str[i].to_i
}
