# 補題C0/定理C1 の挿入DP: A_k(n)/2 (無向ハミルトンパス数) を O(B(k)*n) で計算
# 状態: [d, a, aa, af, sp]
#   d  = def(t) = 2t - E(t)  (<= k-1)
#   a  = 起床済み孤立頂点数
#   aa = 端が (A,A) の断片数, af = (A,F) の断片数
#   sp = 特殊断片(カプセル端 C_s を含む)のソート済みリスト: ["A","C3"], ["F","C2"], ["C1","C2"], ["S3"](孤立カプセル)
class DiagDP
  def initialize(k) @k = k end

  def fcount(st)
    _, _, _, af, sp = st
    af + sp.sum { |fr| fr.count { |tok| tok == "F" } }
  end

  def contrib(st, cls)
    case cls[0]
    when :iso then "A"
    when :aa  then "A"
    when :af  then "F"
    when :sp  then (st[4][cls[1]] - ["A"])[0]
    end
  end

  def consume(st, cls)
    d, a, aa, af, sp = st
    case cls[0]
    when :iso then [d, a-1, aa, af, sp]
    when :aa  then [d, a, aa-1, af, sp]
    when :af  then [d, a, aa, af-1, sp]
    when :sp  then [d, a, aa, af, sp.each_with_index.reject { |_, i| i == cls[1] }.map(&:first)]
    end
  end

  def add_frag(st, fr, final)
    d, a, aa, af, sp = st
    fr = fr.sort
    if fr == ["A", "A"] then [d, a, aa+1, af, sp]
    elsif fr == ["A", "F"] then [d, a, aa, af+1, sp]
    elsif fr == ["F", "F"] then final ? [d, a, aa, af, sp + [fr]] : nil
    else [d, a, aa, af, (sp + [fr]).sort]
    end
  end

  def consume_pair(st, cl1, cl2)
    d, a, aa, af, sp = st
    drops = []
    [cl1, cl2].each do |cl|
      case cl[0]
      when :iso then a -= 1
      when :aa  then aa -= 1
      when :af  then af -= 1
      when :sp  then drops << cl[1]
      end
    end
    sp2 = sp.each_with_index.reject { |_, i| drops.include?(i) }.map(&:first)
    [d, a, aa, af, sp2]
  end

  def targets(st)
    _, a, aa, af, sp = st
    t = []
    t << [[:iso], a] if a > 0
    t << [[:aa], 2*aa] if aa > 0
    t << [[:af], af] if af > 0
    sp.each_with_index { |fr, i| t << [[:sp, i], 1] if fr.include?("A") }
    t
  end

  def step(states, t, n, k)
    w = {}
    states.each do |st, c|
      d, a, aa, af, sp = st
      if t <= n + 1
        add(w, [d, a+1, aa, af, sp], c)
      else
        s = t - n - 1
        if sp.include?(["S#{s}"])
          add(w, [d, a+1, aa, af, sp - [["S#{s}"]]], c)
        elsif (i = sp.index { |fr| fr.include?("C#{s}") })
          fr = (sp[i] - ["C#{s}"] + ["A"]).sort
          rest = sp.each_with_index.reject { |_, j| j == i }.map(&:first)
          st2 = add_frag([d, a, aa, af, rest], fr, false)
          add(w, st2, c) if st2
        else
          add(w, st, c)
        end
      end
    end
    capsule = (t <= k - 2)
    final = (t == n + k - 1)
    out = {}
    w.each do |st, c|
      d = st[0]
      tg = targets(st)
      tg.each_with_index do |(cl1, m1), i|
        tg.each_with_index do |(cl2, m2), j|
          next if j < i
          mult = if i == j
                   case cl1[0]
                   when :iso then m1 * (m1 - 1) / 2
                   when :aa  then st[2] >= 2 ? 2 * st[2] * (st[2] - 1) : 0
                   when :af  then m1 * (m1 - 1) / 2
                   else 0
                   end
                 else m1 * m2 end
          next if mult == 0
          st1 = consume_pair(st, cl1, cl2)
          fr = [contrib(st, cl1), contrib(st, cl2)]
          st2 = add_frag(st1, fr, final)
          add(out, st2, c * mult) if st2 && ok(st2)
        end
      end
      newtok = capsule ? "C#{t}" : "F"
      tg.each do |cl, m|
        st1 = consume(st, cl)
        st1 = [st1[0] + 1, st1[1], st1[2], st1[3], st1[4]]
        st2 = add_frag(st1, [contrib(st, cl), newtok], final)
        add(out, st2, c * m) if st2 && ok(st2)
      end
      if capsule
        st2 = [d + 2, st[1], st[2], st[3], (st[4] + [["S#{t}"]]).sort]
        add(out, st2, c) if ok(st2)
      end
    end
    out
  end

  def ok(st)
    st[0] <= @k - 1 && fcount(st) <= 2
  end

  def add(h, st, c)
    h[st] = (h[st] || 0) + c
  end

  def run(n, want_sets: false)
    k = @k
    states = { [0, 0, 0, 0, []] => 1 }
    sets = []
    (1..n + k - 1).each do |t|
      states = step(states, t, n, k)
      sets << states.keys.sort if want_sets
    end
    total = 0
    states.each do |st, c|
      d, a, aa, af, sp = st
      next unless a == 0 && aa + af + sp.size == 1 && sp.all? { |fr| fr == ["F", "F"] }
      total += c
    end
    want_sets ? [total, sets] : total
  end
end

if $0 == __FILE__
  k = ARGV[0].to_i
  nmax = ARGV[1].to_i
  dp = DiagDP.new(k)
  (k..nmax).each { |n| puts "#{n} #{2 * dp.run(n)}" }
end
