# 認証スクリプト: 定理C1(次数上界) + 補題10.1(認証) による k ごとの完全証明
#   使い方:  ruby certify.rb K [K2 ...]
#   例:      nohup ruby certify.rb 7 > k7.log 2>&1 &
# 生存刈り込み(補題9.1)を組み込み済み。alive_K.marshal に生存集合をキャッシュ。
require_relative 'diagdp'
CK = {
  2 => [1],                        # X-1
  3 => [4, -4],                    # (X-2)^2
  4 => [7, -12],                   # (X-3)(X-4)
  5 => [16, -84, 144],             # (X-4)(X-6)^2
  6 => [22, -157, 360],            # (X-5)(X-8)(X-9)
  7 => [40, -588, 3744, -8640],    # (X-6)(X-10)(X-12)^2
  8 => [50, -913, 7164, -20160],   # (X-7)(X-12)(X-15)(X-16)
  9 => [80, -2508, 38336, -283840, 806400], # (X-8)(X-14)(X-18)(X-20)^2 (k=9は予想の外挿)
}
BIG = 10**9

def head_vec(dp, k)
  u = { [0,0,0,0,[]] => 1 }
  (1..(k-2)).each { |t| u = dp.step(u, t, BIG, k) }
  u
end

def reach_closure(dp, k)
  seen = {}
  queue = head_vec(dp, k).keys
  queue.each { |s| seen[s] = true }
  until queue.empty?
    nxt = dp.step(queue.to_h { |s| [s,1] }, k, BIG, k)
    queue = nxt.keys.reject { |s| seen[s] }
    queue.each { |s| seen[s] = true }
  end
  seen.keys
end

def accepts?(dp, k, s)
  n = BIG
  st = { s => 1 }
  ((n+2)..(n+k-1)).each { |t| st = dp.step(st, t, n, k) }
  st.any? do |x, c|
    _, a, aa, af, sp = x
    a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F","F"] } && c > 0
  end
end

def alive_set(dp, k, states)
  # キャッシュはコード(diagdp.rb)と閉包のフィンガープリントで検証する。
  # 不一致(旧版のキャッシュ・コード変更・閉包変化)なら再計算。旧形式も自動的に不合格。
  require 'digest'
  fp = Digest::SHA256.hexdigest(
    File.binread(File.join(__dir__, 'diagdp.rb')) + Marshal.dump(states.sort_by(&:inspect)))
  file = "alive_#{k}.marshal"
  if File.exist?(file)
    data = Marshal.load(File.read(file)) rescue nil
    return data[1] if data.is_a?(Array) && data[0] == fp
    puts "k=#{k}: キャッシュ #{file} はフィンガープリント不一致のため破棄して再計算"
  end
  succ = {}
  states.each { |s| succ[s] = dp.step({s=>1}, k, BIG, k).keys }
  alive = {}
  states.each { |s| alive[s] = true if accepts?(dp, k, s) }
  loop do
    grew = false
    states.each do |s|
      next if alive[s]
      if succ[s].any? { |x| alive[x] }
        alive[s] = true; grew = true
      end
    end
    break unless grew
  end
  File.write(file, Marshal.dump([fp, alive]))
  alive
end

$stdout.sync = true
ARGV.map(&:to_i).each do |k|
  dp = DiagDP.new(k)
  c = CK[k] or (puts "k=#{k}: CK 未登録"; next)
  d = c.size
  t0 = Time.now
  full = reach_closure(dp, k)
  alive = alive_set(dp, k, full)
  bset = full.select { |s| alive[s] }
  b = bset.size
  puts "k=#{k}: 全閉包 #{full.size} -> 生存 #{b} (次数上界)  [#{(Time.now-t0).round(1)}s]"
  nmax = k + b + d + 8
  puts "k=#{k}: 認証には n=#{k}..#{nmax} の厳密値が必要。生成開始..."

  u = head_vec(dp, k)
  u.select! { |s,_| alive[s] }
  vals = {}
  j = 0
  loop do
    n = j + k - 3
    if n >= k
      tail = u
      ((n+2)..(n+k-1)).each { |t| tail = dp.step(tail, t, n, k) }
      tot = 0
      tail.each do |st, cc|
        _, a, aa, af, sp = st
        next unless a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F","F"] }
        tot += cc
      end
      vals[n] = 2 * tot
      puts "  n=#{n} 完了 [#{(Time.now-t0).round(1)}s]" if n % 200 == 0
    end
    break if n >= nmax
    u = dp.step(u, k, BIG, k)
    u.select! { |s,_| alive[s] }
    j += 1
  end

  ok = ->(n) { vals[n] == (1..d).sum { |i| c[i-1] * vals[n-i] } }
  n_lo = ((k+d)..nmax).find { |n| ((n)..nmax).all? { |m| ok.(m) } }
  win = n_lo ? (nmax - n_lo + 1) : 0
  verdict = (n_lo && win >= b + 1) ? "*** 認証成立 = n>=#{n_lo} で完全証明 ***" : "認証不成立(窓不足または漸化式不成立)"
  puts "k=#{k}: C_k 成立開始 n_lo=#{n_lo.inspect}  窓長 #{win} (必要 #{b+1})  #{verdict}  [#{(Time.now-t0).round(1)}s]"
end
