require 'prime'

def A(n)
  ary = []
  (1..n / 4).each{|i|
    j = 2
    while (k = i * j ** j + 1) <= n
      ary << k if k.prime?
      j += 1
    end
  }
  ary.uniq
end

ary = A(3 * 10 ** 5).sort
(1..10 ** 4).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
