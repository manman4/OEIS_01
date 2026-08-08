require_relative 'diagdp'
# (1) バルク状態集合の閉包 S* を計算 (次数上界 B(k) = |S*|)
# (2) ストリーミングで A_k(n) を多数生成し、C_k 漸化式を窓 [n_lo, n_lo+B+deg] で検証
CK = {
  3 => [4, -4],                    # (X-2)^2
  4 => [7, -12],                   # (X-3)(X-4)
  5 => [16, -84, 144],             # (X-4)(X-6)^2
  6 => [22, -157, 360],            # (X-5)(X-8)(X-9)
  7 => [40, -588, 3744, -8640],    # (X-6)(X-10)(X-12)^2
  8 => [50, -913, 7164, -20160],   # (X-7)(X-12)(X-15)(X-16)
}

def closure(dp, k)
  big = 10**6
  states = { [0,0,0,0,[]] => 1 }
  (1..(k-2)).each { |t| states = dp.step(states, t, big, k) } # 頭部(カプセル生成期)
  supp = states.keys.to_a
  seen = supp.to_h { |s| [s, true] }
  queue = supp.dup
  until queue.empty?
    batch = { }
    queue.each { |s| batch[s] = 1 }
    nxt = dp.step(batch, k, big, k)  # t=k はバルク(wake枝, capsule=false, final=false)
    queue = []
    nxt.each_key { |s| next if seen[s]; seen[s] = true; queue << s }
  end
  seen.keys
end

def terms(dp, k, nmax)
  # 一回のバルク走査で全 n の値を得る: n = j + k - 3 (j = バルク step 数)
  big = 10**9
  states = { [0,0,0,0,[]] => 1 }
  (1..(k-2)).each { |t| states = dp.step(states, t, big, k) }
  vals = {}
  j = 0
  loop do
    n = j + k - 3
    if n >= k
      tail = states
      ((n+2)..(n+k-1)).each { |t| tail = dp.step(tail, t, n, k) }
      tot = 0
      tail.each do |st, c|
        d, a, aa, af, sp = st
        next unless a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F","F"] }
        tot += c
      end
      vals[n] = 2 * tot
    end
    break if n >= nmax
    # バルク1step: t は k-1..n+1 の範囲ならどれでも同一写像。t=k, n=big で代用
    states = dp.step(states, k, big, k)
    j += 1
  end
  vals
end

$stdout.sync = true
ks = (ARGV.empty? ? [3,4,5,6] : ARGV.map(&:to_i))
ks.each do |k|
  dp = DiagDP.new(k)
  t0 = Time.now
  s_star = closure(dp, k)
  b = s_star.size
  coeffs = CK[k]
  d = coeffs.size
  nmax = k + b + d + 8
  vals = terms(dp, k, nmax)
  # 検算: 直接 run と照合 (最初の4項)
  chk = (k..[k+3, nmax].min).all? { |n| vals[n] == 2 * dp.run(n) }
  # 漸化式の成立開始点 n_lo を探す
  ok = ->(n) { vals[n] == (1..d).sum { |i| coeffs[i-1] * vals[n-i] } }
  n_lo = nil
  ((k+d)..nmax).each do |n|
    if ((n)..nmax).all? { |m| ok.(m) }
      n_lo = n; break
    end
  end
  win = n_lo ? (nmax - n_lo + 1) : 0
  puts "k=#{k}: B(k)=|S*|=#{b}  検算=#{chk ? 'OK' : 'NG'}  C_k成立開始 n_lo=#{n_lo}" \
       "  検証窓=[#{n_lo},#{nmax}] 長さ#{win} (必要 #{b + 1})  " \
       "#{win >= b + 1 ? '*** 認証成立 ***' : '窓不足'}  (#{(Time.now-t0).round(1)}s)"
end
