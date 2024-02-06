# f_hashもb_hashもハッシュで、m次の多項式を表わしている
def mul(f_hash, b_hash, m)
  h = Hash.new(0)
  f_hash.each{|f_key, f_val|
    b_hash.each{|b_key, b_val|
      new_key = f_key + b_key
      h[new_key] += f_val * b_val if new_key <= m
    }
  }
  h
end

def A(k, n)
  ary = {0 => 1}
  (1..n / k).each{|i|
    b_ary = {0 => 1}
    (1..n).each{|j| b_ary[j] = 0}
    # k*iからnまでi刻みで1
    (k * i).step(n, i){|j| b_ary[j] = 1}
    ary = mul(ary, b_ary, n)
  }
  ary
end

n = 5100
m = 5000
ary = A(5, n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
