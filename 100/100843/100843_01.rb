n = 35
@part = [1, 1]
def part(i)
  sum = 0
  (0..i).each{|j|
    i1 = (1 - 2 * (j % 2)) * ((j + 2) / 2) # 1, -1, 2, -2, 3, -3, ...
    i1 = i1 * (3 * i1 - 1) / 2             # 1, 2, 5, 7, 12, 15, ...
    break if i1 > i
    if (j / 2).even?
      sum += @part[i - i1]
    else
      sum -= @part[i - i1]
    end
  }
  @part[i] = sum
end
(1..n).each{|i| part(i)}

max = @part[-1]
i, j = 0, 1
ary = [i, j]
cnt = 1
while cnt < max
  i, j = j, i + j
  ary << j
  cnt += 1
end
(1..n).each{|i|
  j = ary[@part[i]]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
