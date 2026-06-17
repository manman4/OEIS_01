def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(1) = 1; a(n) = Sum_{i,j,k >= 0 and i+j+k=n-2} (n-1)!/((i+1)!*j!*k!) * a(i+1)*a(j+1)*a(k+1).
def a(n)
  ary = [0, 1]
  (2..n).each{|nn|
    s = 0
    (0..nn-2).each{|i|
      (0..nn-2-i).each{|j|
        k = nn-2-i-j
        s += f(nn-1)/((f(i+1)*f(j)*f(k))) * ary[i+1]*ary[j+1]*ary[k+1]
      }
    }
    ary << s
  }
  ary
end

p a(20)[1..-1]