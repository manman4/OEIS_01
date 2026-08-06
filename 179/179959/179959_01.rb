# frozen_string_literal: true
#
# a_k(n) = number of permutations p of [n] with |p(i+1)-p(i)| >= k for 1 <= i < n.
#
# Inclusion-exclusion:
#   a_k(n) = Sum_F (-1)^e(F) * 2^c(F) * (n-e(F))!
# where F ranges over the linear forests of the graph G_{n,k} on [n]
# with edges {u,v} satisfying 0 < |u-v| < k, and e(F), c(F) are the numbers
# of edges and nontrivial components of F.
#
# Since G_{n,k} has bandwidth at most k-1, the weighted linear forests are
# counted by a transfer matrix on a frontier of the last k-1 vertices.
#
# Weight trick: for a linear forest, c(F) = v(F) - e(F) where v(F) is the number
# of vertices of degree >= 1.  So the DP carries a polynomial in x marking edges
# and multiplies by 2 for each covered vertex; the coefficient of x^j comes out
# as 2^j * F_j, so the division back is exact and everything stays in integers.

module PermGap
  module_function

  # ---- frontier DP -------------------------------------------------------
  #
  # A state is an array of slots, one per frontier vertex, in vertex order.
  # Each slot is [deg, label]:
  #   deg   = 0, 1 or 2  (degree so far inside the forest)
  #   label = component id for deg-1 vertices (open path ends), nil otherwise.
  # A label occurring twice means both ends of that path are still in the
  # frontier; occurring once means the other end is already closed off.
  # deg-2 vertices need no label: they can never take another edge.

  # Returns [a_k(1), a_k(2), ..., a_k(nmax)].
  def series(k, nmax)
    raise ArgumentError, 'k >= 1' if k < 1
    w = k - 1                       # frontier width
    states = { [] => [1] }          # state -> polynomial, coeff[j] = weight of j-edge forests
    out = []

    (1..nmax).each do |_m|
      # add the new vertex, choosing 0, 1 or 2 edges back into the frontier
      nxt = {}
      states.each do |st, poly|
        extensions(st).each { |nst, added| accumulate(nxt, nst, shift(poly, added)) }
      end

      # the oldest frontier vertex is now out of reach: retire it
      if !nxt.empty? && nxt.keys.first.size > w
        shrunk = {}
        nxt.each do |st, poly|
          p = st[0][0] >= 1 ? poly.map { |c| c * 2 } : poly
          accumulate(shrunk, canon(st[1..]), p)
        end
        nxt = shrunk
      end
      states = nxt

      # read off F_{m,k,*}: vertices still in the frontier have not been paid for yet
      total = []
      states.each do |st, poly|
        f = 1 << st.count { |d, _| d >= 1 }
        poly.each_with_index { |c, j| total[j] = (total[j] || 0) + c * f }
      end
      out << assemble(out.size + 1, total)
    end
    out
  end

  def value(k, n)
    series(k, n).last
  end

  # all ways to append one new vertex, returning [new_state, edges_added]
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
        else                                      # new vertex passes the open end on
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

  def shift(poly, k)
    k.zero? ? poly : Array.new(k, 0) + poly
  end

  def accumulate(h, key, poly)
    cur = h[key] ||= []
    poly.each_with_index { |c, j| cur[j] = (cur[j] || 0) + c }
  end

  # Sum_j (-1)^j F_j (n-j)!,  with F_j = total[j] / 2^j
  def assemble(n, total)
    fact = [1]
    (1..n).each { |i| fact[i] = fact[i - 1] * i }
    s = 0
    total.each_with_index do |c, j|
      next if c.nil? || c.zero?
      s += (j.even? ? 1 : -1) * (c / (1 << j)) * fact[n - j]
    end
    s
  end

  # ---- brute force, for checking ----------------------------------------
  def brute(k, n)
    return 1 if n <= 1
    (1..n).to_a.permutation.count do |p|
      (0...n - 1).all? { |i| (p[i + 1] - p[i]).abs >= k }
    end
  end
end

if $PROGRAM_NAME == __FILE__
  if ARGV.empty?
    warn "usage: ruby #{__FILE__} K NMAX"
    exit 1
  end
  k, nmax = ARGV[0].to_i, (ARGV[1] || 20).to_i
  PermGap.series(k, nmax).each_with_index { |v, i| puts "a_#{k}(#{i + 1}) = #{v}" }
end
