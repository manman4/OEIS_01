# aval.rb — 対角族 A_k(n) = #{[2n+k] の順列 p : |p(i+1)-p(i)| > n} を出力
#
# 使い方:
#   ruby aval.rb K NMAX        # n = 0..NMAX
#   ruby aval.rb K N0 NMAX     # n = N0..NMAX
#
# 計算方法:
#   n <= k-3 : perm_gap.rb の固定ギャップ DP で A_k(n) = a_{n+1}(2n+k) を直接計算
#   n >= k-2 : diagdp.rb の挿入 DP を一掃き(バルクを1stepずつ進め、各 n で尾部+受理)
# 両者は独立実装であり、k=7,9 の境界点で相互一致を確認済み(proved_results.md §8, §10)。
# 長大な範囲(数百項以上)には certify.rb と同様の生存刈り込みが有効だが、
# 本スクリプトは簡明さを優先して刈り込みなしで計算する(結果は同一)。
require_relative 'diagdp'
require_relative 'perm_gap'

if ARGV.size < 2
  warn "usage: ruby #{__FILE__} K [N0] NMAX"
  exit 1
end
k = ARGV[0].to_i
n0, nmax = ARGV.size >= 3 ? [ARGV[1].to_i, ARGV[2].to_i] : [0, ARGV[1].to_i]
abort "k >= 2 としてください" if k < 2
abort "0 <= N0 <= NMAX としてください" if n0 < 0 || n0 > nmax
$stdout.sync = true

# --- 小さい n: 固定ギャップ DP ---
(n0..[nmax, k - 3].min).each do |n|
  v = PermGap.series(n + 1, 2 * n + k).last
  puts "#{n} #{v}"
end

# --- n >= k-2: 挿入 DP の一掃き ---
lo = [n0, k - 2].max
if lo <= nmax
  dp = DiagDP.new(k)
  big = 10**9
  u = { [0, 0, 0, 0, []] => 1 }
  (1..(k - 2)).each { |t| u = dp.step(u, t, big, k) }   # 頭部
  j = 0
  loop do
    n = j + k - 3
    if n >= lo && n >= k - 2
      tail = u
      ((n + 2)..(n + k - 1)).each { |t| tail = dp.step(tail, t, n, k) }
      tot = 0
      tail.each do |st, c|
        _, a, aa, af, sp = st
        next unless a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F", "F"] }
        tot += c
      end
      puts "#{n} #{2 * tot}"
    end
    break if n >= nmax
    u = dp.step(u, k, big, k)                            # バルク1step
    j += 1
  end
end
