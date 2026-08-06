# frozen_string_literal: true
#
# a_k(n) = number of permutations p of [n] with |p(i+1)-p(i)| >= k for 1 <= i < n.
#
#   a_k(n) = Sum_F (-1)^e(F) * 2^c(F) * (n-e(F))!
#
# F ranges over the linear forests of the graph G_{n,k} on [n] with edges {u,v}
# satisfying 0 < |u-v| < k; e(F) and c(F) are the numbers of edges and nontrivial
# components of F.  G_{n,k} has bandwidth at most k-1, so a transfer matrix whose
# size depends only on k counts the weighted forests.
#
# Weight trick: for a linear forest c(F) = v(F) - e(F), where v(F) counts vertices
# of degree >= 1.  The DP carries a polynomial in x marking edges and multiplies by
# 2 per covered vertex; the coefficient of x^j comes out as 2^j * F_j, so dividing
# back is exact and everything stays in integers.
#
# The transfer matrix depends only on k, so it is built once (Machine) and cached.

module PermGap
  module_function

  # ---- states ------------------------------------------------------------
  #
  # A state is an array of slots, one per frontier vertex, in vertex order.
  # Slot = [deg, label]: deg in {0,1,2}; label identifies the path component for
  # deg-1 vertices (open path ends), nil otherwise.  A label occurring twice means
  # both ends of that path are still in the frontier, once means the other end is
  # already closed.  deg-2 vertices need no label: they can take no further edge.

  # all ways to append one new vertex, as [new_state, edges_added]
  def extensions(st)
    res = [[st + [[0, nil]], 0]]
    n = st.size
    (0...n).each do |i|
      s = link(st, [i])
      res << [s, 1] if s
    end
    (0...n).each do |i|
      (i + 1...n).each do |j|
        s = link(st, [i, j])
        res << [s, 2] if s
      end
    end
    res
  end

  # attach the new vertex to the frontier positions in idxs; nil if illegal
  def link(st, idxs)
    deg = st.map { |d, _| d } << 0
    lab = st.map { |_, l| l } << nil
    me  = deg.size - 1
    nextlab = (lab.compact.max || -1) + 1

    idxs.each do |i|
      return nil if deg[i] >= 2 || deg[me] >= 2   # no branching
      if deg[i].zero?
        if deg[me].zero?                          # brand new path of one edge
          l = nextlab; nextlab += 1
          deg[i] = 1; lab[i] = l
          deg[me] = 1; lab[me] = l
        else                                      # new vertex hands the open end on
          deg[i] = 1; lab[i] = lab[me]
          deg[me] = 2; lab[me] = nil
        end
      else                                        # deg[i] == 1: extend that path
        li = lab[i]
        if deg[me].zero?
          deg[i] = 2; lab[i] = nil
          deg[me] = 1; lab[me] = li
        else
          lm = lab[me]
          return nil if lm == li                  # would close a cycle
          deg[i] = 2; lab[i] = nil
          deg[me] = 2; lab[me] = nil
          lab.each_index { |t| lab[t] = lm if lab[t] == li }  # merge the two paths
        end
      end
    end
    canon(deg.zip(lab))
  end

  # relabel components 0,1,2,... in order of first appearance
  def canon(slots)
    map = {}
    slots.map { |d, l| [d, l.nil? ? nil : (map[l] ||= map.size)] }
  end

  # ---- transfer matrix, built once per k ---------------------------------
  class Machine
    attr_reader :trans, :readoff, :size

    def initialize(k)
      w = k - 1
      index = { [] => 0 }
      states = [[]]
      @trans = []
      cur = 0
      while cur < states.size
        st = states[cur]
        list = []
        PermGap.extensions(st).each do |ns, edges|
          factor = 1
          if ns.size > w                       # retire the oldest frontier vertex
            factor = ns[0][0] >= 1 ? 2 : 1     # pay for it if it is covered
            ns = PermGap.canon(ns[1..])
          end
          j = index[ns]
          unless j
            j = states.size
            index[ns] = j
            states << ns
          end
          list << [j, edges, factor]
        end
        @trans[cur] = list
        cur += 1
      end
      # vertices still in the frontier have not been paid for yet
      @readoff = states.map { |st| 1 << st.count { |d, _| d >= 1 } }
      @size = states.size
    end
  end

  MACHINES = {}
  def machine(k)
    MACHINES[k] ||= Machine.new(k)
  end

  # ---- the sequence ------------------------------------------------------

  # Returns [a_k(0), a_k(1), ..., a_k(nmax)], so that series(k, N)[n] == a_k(n).
  # a_k(0) = 1: the empty permutation vacuously satisfies the gap condition, and
  # the formula gives it too (only the empty forest, contributing 0! = 1).
  def series(k, nmax)
    raise ArgumentError, 'k >= 1' if k < 1
    raise ArgumentError, 'nmax >= 0' if nmax < 0
    mac = machine(k)
    polys = Array.new(mac.size)
    polys[0] = [1]                      # empty frontier, no vertices placed yet
    fact = [1]
    (1..nmax).each { |i| fact[i] = fact[i - 1] * i }
    out = [assemble(mac, polys, 0, fact)]

    (1..nmax).each do |n|
      nxt = Array.new(mac.size)
      polys.each_with_index do |p, i|
        next unless p
        mac.trans[i].each do |j, edges, factor|
          t = (nxt[j] ||= [])
          p.each_with_index do |c, d|
            next if c.nil? || c.zero?
            idx = d + edges
            t[idx] = (t[idx] || 0) + c * factor
          end
        end
      end
      polys = nxt
      out << assemble(mac, polys, n, fact)
    end
    out
  end

  # Sum_j (-1)^j F_j (n-j)!, reading F_j off the current DP layer
  def assemble(mac, polys, n, fact)
    total = []
    polys.each_with_index do |p, i|
      next unless p
      f = mac.readoff[i]
      p.each_with_index { |c, j| total[j] = (total[j] || 0) + c * f unless c.nil? }
    end
    s = 0
    total.each_with_index do |c, j|
      next if c.nil? || c.zero?
      s += (j.even? ? 1 : -1) * (c >> j) * fact[n - j]   # c is exactly 2^j * F_j
    end
    s
  end

  def value(k, n)
    series(k, n).last
  end

  # ---- brute force, for checking ----------------------------------------

  # backtracking: prunes as soon as a gap is too small
  def brute(k, n)
    return 1 if n <= 1
    used = Array.new(n + 1, false)
    count = 0
    walk = lambda do |prev, depth|
      if depth == n
        count += 1
        return
      end
      (1..n).each do |v|
        next if used[v]
        next if prev && (v - prev).abs < k
        used[v] = true
        walk.call(v, depth + 1)
        used[v] = false
      end
    end
    walk.call(nil, 0)
    count
  end
end

if $PROGRAM_NAME == __FILE__
  if ARGV.empty?
    warn "usage: ruby #{__FILE__} K NMAX"
    exit 1
  end
  k, nmax = ARGV[0].to_i, (ARGV[1] || 20).to_i
  PermGap.series(k, nmax).each{|v|
    print "#{v}, "
  }
end
