# mŸˆÈ‰º‚ğæ‚èo‚·
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

def A137569(n)
  ary1 = [1]
  1.step(n, 3).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary1 = mul(ary1, b_ary, n)
  }
  ary2 = [1]
  2.step(n, 3).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary2 = mul(ary2, b_ary, n)
  }
  mul(ary1, ary2, n)
end

n = 1000
ary = A137569(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
