def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(0) = 1; a(n) = Sum_{k=0..floor((n-1)/3)} a(3*k) * a(n-1-3*k).
def a(n, x)
  p x
  xs = x.size
  ary = [1]
  (1..n).each{|m|
    s = 0
    (0..m - 1).to_a.repeated_permutation(xs).each{|i|
      # x[k] * i[k]
      y = i.map.with_index{|v, k| x[k] * v}
      s += (1..xs).inject(1){|prod , k| prod * ary[y[k - 1]]} if y.sum == m - 1
    }
    ary << s
  }
  ary
end

n = 34
p a(n, [1, 3])
p a(n, [1, 2, 3]) # A385699
