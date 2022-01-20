def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end

def f(n)
  cnt = 0
  partition(n, 1, n).each{|ary|
    cnt += 1 if ary[-1] % ary.size == 0
  }
  cnt
end

p (1..20).map{|i| f(i)}