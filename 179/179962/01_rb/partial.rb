require_relative 'diagdp'
CK = { 7 => [40,-588,3744,-8640], 8 => [50,-913,7164,-20160] }
BIG = 10**9
$stdout.sync = true
[[7, 120], [8, 45]].each do |k, nmax|
  t0 = Time.now
  dp = DiagDP.new(k)
  alive = Marshal.load(File.read("alive_#{k}.marshal"))
  u = { [0,0,0,0,[]] => 1 }
  (1..(k-2)).each { |t| u = dp.step(u, t, BIG, k) }
  u.select! { |s,_| alive[s] }
  vals = {}
  j = 0
  loop do
    n = j + k - 3
    if n >= k
      tail = u
      ((n+2)..(n+k-1)).each { |t| tail = dp.step(tail, t, n, k) }
      tot = 0
      tail.each do |st, c|
        d2, a, aa, af, sp = st
        next unless a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F","F"] }
        tot += c
      end
      vals[n] = 2 * tot
    end
    break if n >= nmax
    u = dp.step(u, k, BIG, k)
    u.select! { |s,_| alive[s] }
    j += 1
  end
  c = CK[k]; d = c.size
  bad = ((k+d)..nmax).reject { |n| vals[n] == (1..d).sum { |i| c[i-1]*vals[n-i] } }
  puts "k=#{k}: 厳密値 n=#{k}..#{nmax} 生成。C_k 漸化式: " \
       "#{bad.empty? ? "n=#{k+d}..#{nmax} の全点で成立" : "不成立点 #{bad.inspect}"}  (#{(Time.now-t0).round(1)}s)"
  puts "  照合(既知): A_7(7)=#{vals[7] if k==7}  A_8(8)=#{vals[8] if k==8}"
end
