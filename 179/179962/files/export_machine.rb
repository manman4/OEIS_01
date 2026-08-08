# encoding: utf-8
# export_machine.rb — 挿入 DP の転移表を C 版(diag.c)向けに書き出す
#
#   ruby export_machine.rb K [OUTFILE]
#
# 出力形式(テキスト):
#   k <k>
#   head <len>            ; 頭部(t=1..k-2)適用後のベクトル: (状態index, 係数)
#   <idx> <coef>  x len
#   bulk <nstates> <nnz>  ; バルク1step: 疎行列 (from to mult)
#   <from> <to> <mult>  x nnz
#   tail <layers>         ; 尾部 k-2 層。層ごとに次元が変わる
#   layer <rows> <cols> <nnz>
#   <from> <to> <mult>  x nnz
#   accept <len>          ; 最終層での受理状態index
#   <idx>  x len
#
# 状態は生存刈り込み(補題9.1)後に 0..nstates-1 へ再付番する。
require_relative 'diagdp'

BIG = 10**9

def head_vec(dp, k)
  u = { [0, 0, 0, 0, []] => 1 }
  (1..(k - 2)).each { |t| u = dp.step(u, t, BIG, k) }
  u
end

def reach_closure(dp, k)
  seen = {}
  queue = head_vec(dp, k).keys
  queue.each { |s| seen[s] = true }
  until queue.empty?
    nxt = dp.step(queue.to_h { |s| [s, 1] }, k, BIG, k)
    queue = nxt.keys.reject { |s| seen[s] }
    queue.each { |s| seen[s] = true }
  end
  seen.keys
end

def accepting?(st)
  _, a, aa, af, sp = st
  a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F", "F"] }
end

def alive_set(dp, k, states)
  succ = states.to_h { |s| [s, dp.step({ s => 1 }, k, BIG, k).keys] }
  # 受理可能性: 尾部を通して受理に至れるか
  alive = {}
  states.each do |s|
    st = { s => 1 }
    (1..(k - 2)).each { |j| st = dp.step(st, BIG + 1 + j, BIG, k) }
    alive[s] = true if st.any? { |x, c| accepting?(x) && c > 0 }
  end
  loop do
    grew = false
    states.each do |s|
      next if alive[s]
      if succ[s].any? { |x| alive[x] }
        alive[s] = true
        grew = true
      end
    end
    break unless grew
  end
  alive
end

k = (ARGV[0] || 4).to_i
abort "k >= 3 としてください" if k < 3
out = ARGV[1] || "machine_k#{k}.txt"
$stdout.sync = true

dp = DiagDP.new(k)
full = reach_closure(dp, k)
alive = alive_set(dp, k, full)
bulk_states = full.select { |s| alive[s] }.sort_by(&:inspect)
idx = bulk_states.each_with_index.to_h
warn "k=#{k}: 全閉包 #{full.size} -> 生存 #{bulk_states.size}"

lines = []
lines << "k #{k}"

hv = head_vec(dp, k).select { |s, _| alive[s] }
lines << "head #{hv.size}"
hv.each { |s, c| lines << "#{idx[s]} #{c}" }

bulk = []
bulk_states.each do |s|
  dp.step({ s => 1 }, k, BIG, k).each do |t, m|
    next unless alive[t]
    bulk << [idx[s], idx[t], m]
  end
end
lines << "bulk #{bulk_states.size} #{bulk.size}"
bulk.each { |f, t, m| lines << "#{f} #{t} #{m}" }

# 尾部: 層ごとに状態空間が変わるので層別に付番
lines << "tail #{k - 2}"
cur_states = bulk_states
cur_idx = idx
(1..(k - 2)).each do |j|
  nxt_index = {}
  nxt_states = []
  edges = []
  cur_states.each do |s|
    dp.step({ s => 1 }, BIG + 1 + j, BIG, k).each do |t, m|
      ti = nxt_index[t]
      unless ti
        ti = nxt_states.size
        nxt_index[t] = ti
        nxt_states << t
      end
      edges << [cur_idx[s], ti, m]
    end
  end
  lines << "layer #{cur_states.size} #{nxt_states.size} #{edges.size}"
  edges.each { |f, t, m| lines << "#{f} #{t} #{m}" }
  cur_states = nxt_states
  cur_idx = nxt_index
end

acc = cur_states.each_with_index.select { |s, _| accepting?(s) }.map { |_, i| i }
lines << "accept #{acc.size}"
acc.each { |i| lines << i.to_s }

File.write(out, lines.join("\n") + "\n")
warn "k=#{k}: #{out} を書き出しました (bulk #{bulk_states.size} 状態, #{bulk.size} 非零)"
