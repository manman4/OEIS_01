def phi(n)
  phi_ary = [0]
  (1..n).each{|i| phi_ary << i}
  (2..n).each{|p|
    if phi_ary[p] == p
      p.step(n, p){|d| phi_ary[d] -= phi_ary[d] / p}
    end
  }
  phi_ary
end

def A(n)
  phi_ary = phi(n)
  ary = [0]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += phi_ary[i / j] ** (j - 1) if i % j == 0
    }
    ary << s
  }
  ary
end

n = 5000
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}