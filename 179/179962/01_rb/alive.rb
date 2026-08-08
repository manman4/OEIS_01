require_relative 'diagdp'
# 生存(共到達)状態の計算と、刈り込み後の認証
CK = { 3 => [4,-4], 4 => [7,-12], 5 => [16,-84,144],
       6 => [22,-157,360], 7 => [40,-588,3744,-8640], 8 => [50,-913,7164,-20160] }
BIG = 10**9

def head(dp, k)
  u = { [0,0,0,0,[]] => 1 }
  (1..(k-2)).each { |t| u = dp.step(u, t, BIG, k) }
  u
end

def reach_closure(dp, k)
  seen = {}; queue = head(dp, k).keys
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
    d, a, aa, af, sp = x
    a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F","F"] } && c > 0
  end
end

def alive_set(dp, k, states)
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
  alive
end

$stdout.sync = true
(ARGV.empty? ? [6,7,8] : ARGV.map(&:to_i)).each do |k|
  t0 = Time.now
  dp = DiagDP.new(k)
  full = reach_closure(dp, k)
  al = alive_set(dp, k, full)
  na = full.count { |s| al[s] }
  puts "k=#{k}: 全閉包 #{full.size} -> 生存 #{na}  (#{(Time.now-t0).round(1)}s)"
  File.write("alive_#{k}.marshal", Marshal.dump(al))
end
