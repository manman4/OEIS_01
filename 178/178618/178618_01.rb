def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n,k) = Sum_{j=0..k} (-1)^(k-j) * binomial(n+1,k-j) * binomial(n+3*j,3*j).
def T(n, k)
  (0..k).inject(0){|sum, j| sum + (-1)**(k - j) * ncr(n + 1, k - j) * ncr(n + 3 * j, 3 * j)}
end

def A(n)
  # 要素0を除去
  (0..n).map{|i| (0..i).map{|j| T(i, j)}}.flatten.reject(&:zero?)
end

p A(13)