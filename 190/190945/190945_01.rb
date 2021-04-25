def C(n, r)
  r = [r, n - r].min
  return 1 if r == 0
  return n if r == 1
  numerator = (n - r + 1..n).to_a
  denominator = (1..r).to_a
  (2..r).each{|p|
    pivot = denominator[p - 1]
    if pivot > 1
      offset = (n - r) % p
      (p - 1).step(r - 1, p){|k|
        numerator[k - offset] /= pivot
        denominator[k] /= pivot
      }
    end
  }
  result = 1
  (0..r - 1).each{|k|
    result *= numerator[k] if numerator[k] > 1
  }
  return result
end

def A190945(n)
  puts '1 1'
  a = [1]
  (2..n).each{|i|
    b = []
    s = (i - 1) * i / 2 + 1
    (0..s - 1).each{|j|
      x = 0
      (0..(i - 1) * (i - 2) / 2).each{|k|
        d = j - k
        (0..k).each{|m|
          if d + m >= 0
            mm = i - d - 2 * m
            x += a[k] * C(k, m) * C(s - k, mm) * C(i - 1, d + m) if 0 <= mm && m + mm != 0
          end
        }
      }
      b[j] = x
    }
    a = b
    a0 = a[0]
    break if a0.to_s.size > 1000
    print i
    print ' '
    puts a0
  }
end

A190945(100)
