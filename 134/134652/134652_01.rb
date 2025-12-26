t = Rational(1, 1)

(2..20000).each{|n|
  t += Rational((1 << n) - 1, n)
  if t.numerator.gcd(n) > 1
    puts(n - 1)
  end
}