# Numbers k such that k^2 contains exactly 2 copies of each digit of k.
def isok(n)
  (n.to_s.split('') * 2).sort.join == (n * n).to_s.split('').sort.join
end

cnt = 1
(1..10 ** 9).each{|i|
  if isok(i)
    print cnt
    print ' '
    puts i
    cnt += 1
  end
}
