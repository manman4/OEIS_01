class A100471Fast
  def initialize
    # memo[last_d][last_f][n]
    @memo = {}
    @result_memo = { 0 => 1 }
  end

  def value(n)
    cached = @result_memo[n]
    return cached unless cached.nil?
    v = solve(n, n + 1, 0)
    @result_memo[n] = v
    v
  end

  private

  def solve(n, last_d, last_f)
    return 1 if n == 0
    return 0 if last_d <= 1

    min_f = last_f + 1
    # 最小候補 d=1, f=min_f でも届かないなら不可能
    return 0 if n < min_f

    by_d = (@memo[last_d] ||= {})
    by_f = (by_d[last_f] ||= {})
    cached = by_f[n]
    return cached unless cached.nil?

    count = 0
    d = 1
    while d < last_d
      max_f = n / d
      f = min_f
      while f <= max_f
        rest = n - (d * f)
        # 次状態の最小必要値より小さくなったら以降の f は不可能
        break if rest > 0 && rest < (f + 1)
        count += solve(rest, d, f)
        f += 1
      end
      d += 1
    end

    by_f[n] = count
  end
end

n = 500
solver = A100471Fast.new
(0..n).each{|i|
  print i
  print ' '
  puts solver.value(i)
}
