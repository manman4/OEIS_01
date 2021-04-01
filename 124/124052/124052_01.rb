require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# x > 0
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
end

def A(n)
  s = 1
  Prime.each(n){|i|
    cnt = 1
    j = i * i
    while j <= n
      cnt += 1
      j *= i
    end
    s *= (power0(i, cnt + 1) - 1) / (i - 1)
  }
  s
end

(1..2000).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
