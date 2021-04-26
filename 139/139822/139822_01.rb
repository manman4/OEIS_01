require 'prime'

# sort‚µ‚Ä‚¢‚È‚¢
def divisor_list(n)
  return [1] if n == 1
  n.prime_division.map{|e| (0..e[-1]).map{|v| e[0] ** v}}.inject{|res, e| res.map{|t| e.map{|v| t * v}}.flatten}
end

def f(n)
  s = 1
  divisor_list(n).each{|i|
    s *= i + 1 if (i + 1).prime?
  }
  s
end

def A139822(n)
  (0..n).map{|i| f(10 ** i)}
end

n = 20
ary = A139822(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
