alias Ary = Int32 | Array(Ary)

def check(d, a, i)
  return true if i == 0
  (a[i] - a[i - 1]).abs >= d
end

def solve(d, len, a = [] of Int32)
  b = [] of Ary
  if a.size == len
    b << a
  else
    (1..len).each{|m|
      s = a.size
      if s == 0 || (s > 0 && !a.includes?(m))
        if check(d, a + [m], s)
          b += solve(d, len, a + [m])
        end
      end
    }
  end
  b
end

def a179959(n)
  (0..n).map{|i| p [i, solve(6, i).size]}
end

p a179959(14)
