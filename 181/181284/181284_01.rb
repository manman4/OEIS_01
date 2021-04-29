require 'bigdecimal'

i = (9 * 100 ** 30 + 112 - 44 * 30) / 121r
m = 52000
n = m - 2000
str = BigDecimal.new(i, m).sqrt(m).to_s("F")
j = 30
str[0..n - 29].split('').each{|i|
  if i != '.'
    print j
    print ' '
    p i.to_i
    j += 1
  end
}
