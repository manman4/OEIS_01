require 'prime'

def A(n)
  cnt = 0
  s = 1
  while s < n
    cnt += 1 if (n - s).prime?
    s *= 2
  end
  cnt
end

def B(n)
  s = 1
  while A(s) != n
    s += 1
  end
  s
end

p (0..10).map{|i| B(i)}