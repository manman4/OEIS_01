require 'prime'

def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power(a, (n + 1) * x) - 1) / (power(a, x) - 1)}
  sum
end

def A000593(n)
  ary = []
  (1..n).each{|i|
    if i % 2 == 0
      ary << sigma(1, i) - 2 * sigma(1, i / 2)
    else
      ary << sigma(1, i)
    end
  }
  ary
end

def A004011(n)
  [1] + A000593(n).map{|i| i * 24}
end

def A103640(n)
  ary = A004011(n)
  (0..n).map{|i| (-1) ** (i %2) * ary[i]}
end

n = 10000
ary = A103640(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
