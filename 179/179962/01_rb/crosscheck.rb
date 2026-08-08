# perm_gap (包除+frontier DP) と diagdp (挿入DP) の独立クロスチェック
#   A_k(n) = a_{n+1}(2n+k)
# 大きい n では perm_gap 側の状態数(ギャップ n+1 に依存)が爆発するので、
# 「小さい n」ではなく「両者が現実的に走る範囲」で最大限に重ねる。
require_relative 'diagdp'
require_relative 'perm_gap'
$stdout.sync = true
puts "  k   n   perm_gap (包除)          diagdp (挿入DP)          判定"
[[3,2],[3,3],[3,4],[3,5],[4,3],[4,4],[4,5],[5,3],[5,4],[5,5],[6,4],[6,5],[7,5],[7,6],[7,7],[8,6],[8,7]].each do |k, n|
  t0 = Time.now
  a = PermGap.series(n+1, 2*n+k).last
  b = 2 * DiagDP.new(k).run(n)
  printf("  %d  %2d   %-22d  %-22d  %s  [%.1fs]\n", k, n, a, b, a == b ? "一致" : "不一致 ***", Time.now-t0)
end
