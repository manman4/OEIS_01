def ncr(n, r)
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

def A125177(n)
  i = 1
  a = [1, 1]
  ary = [1, 1, 1]
  while ary.size < n + 1
    i += 1
    b = [ncr(2 * i, i) / (i + 1)] + (1..a.size - 1).map{|i| a[i - 1] + a[i]} + [1]
    a = b
    ary += a
  end
  ary[0..n]
end

n = 100
ary = A125177(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
