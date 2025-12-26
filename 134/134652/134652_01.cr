require "big"

t = BigRational.new(1, 1)

(2..100000000).each{|n|
  t += BigRational.new((BigInt.new(1) << n) - 1, n)
  if t.numerator.gcd(n) > 1
    puts n - 1
  end
}
