# A124808: Number of numbers k <= n such that k^2 + 1 is squarefree.
# n = 1..10000 を計算し、b124808_01.txt に "n a(n)" 形式で保存する。
#
# アルゴリズム:
#   k^2 + 1 を割る素数は 2 か p ≡ 1 (mod 4) のみ。また 4 は k^2+1 を割らない。
#   よって k^2+1 が平方因子を持つのは、ある素数 p ≡ 1 (mod 4) について
#   p^2 | k^2+1 となるときに限る。p^2 <= N^2+1 なので p <= N で十分。
#   各 p について x^2 ≡ -1 (mod p) の解 r を求め、Hensel の補題で
#   mod p^2 の解 R に持ち上げ、k ≡ ±R (mod p^2) を篩で除外する。
#   最後に累積和をとる。

N = 10000

# エラトステネスの篩
is_prime = Array.new(N + 1, true)
is_prime[0] = is_prime[1] = false
(2..Integer.sqrt(N)).each do |i|
  next unless is_prime[i]
  (i * i).step(N, i) { |j| is_prime[j] = false }
end

squarefree = Array.new(N + 1, true)
squarefree[0] = false

(5..N).each do |p|
  next unless is_prime[p] && p % 4 == 1
  # x^2 ≡ -1 (mod p) の解
  r = (1...p).find { |x| (x * x + 1) % p == 0 }
  # Hensel: R = r - (r^2+1) / (2r)  (mod p^2)
  m = p * p
  inv = (2 * r).pow(m - p - 1, m) # φ(p^2) = p(p-1)
  rr = (r - (r * r + 1) * inv) % m
  [rr, m - rr].each do |s|
    s.step(N, m) { |k| squarefree[k] = false if k >= 1 }
  end
end

File.open('b124808_01.txt', 'w'){|f|
  count = 0
  (1..N).each{|n|
    count += 1 if squarefree[n]
    f.puts "#{n} #{count}"
  }
}
