def f(k, n)
  return 1 if k == 0
  (n + 1..n + k).inject(:*)
end

def S(k, n)
  (1..n).inject(1){|s, i| s + (0..i - 1).inject(1){|t, j| t * f(k, n - 1 - j) / f(k, j).to_r}}.to_i
end

def A104263(n)
  (0..n).inject(0){|s, i| s + S(i, n - i)}
end

n = 500
(0..n).each{|i|
  j = A104263(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
