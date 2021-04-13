require 'prime'

def f(n)
  n.prime_division.inject(1){|s, i| s * i[0]}
end

def s(n)
  s = 0
  (1..n).each{|i| s += f(i) if n % i == 0}
  s
end

def S0(n)
  [0] + (1..n).map{|i| s(i)}
end

def S1(n)
  ary = [0] + Array.new(n, 0)
  i = 1
  while i <= n
    a = f(i)
    ary[i] += a
    j = i + i
    while j <= n
      ary[j] += a
      j += i
    end
    i += 1
  end
  ary
end

n = 100
ary = S1(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
