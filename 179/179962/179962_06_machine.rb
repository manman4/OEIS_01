#!/usr/bin/env ruby
# frozen_string_literal: true

# Build the finite insertion machine consumed by 179962_06.c.
#
# This is deliberately a readable reference implementation of the state
# transition proved in the large comment at the head of 179962_06.c.  It does
# not use, fit, or assume any scalar recurrence for A_k(n).
#
# Usage:
#   ruby 179962_06_machine.rb 9
#   ruby 179962_06_machine.rb 9 another.machine
#   ruby 179962_06_machine.rb --self-check

class InsertionMachine
  BIG = 1_000_000_000

  def initialize(k)
    @k = k
  end

  def frozen_ends(state)
    _, _, _, af, special = state
    af + special.sum { |fragment| fragment.count("F") }
  end

  def contribution(state, target)
    case target[0]
    when :isolated, :aa then "A"
    when :af then "F"
    when :special then (state[4][target[1]] - ["A"])[0]
    else raise "bad target"
    end
  end

  def consume(state, target)
    d, isolated, aa, af, special = state
    case target[0]
    when :isolated then [d, isolated - 1, aa, af, special]
    when :aa       then [d, isolated, aa - 1, af, special]
    when :af       then [d, isolated, aa, af - 1, special]
    when :special
      rest = special.each_with_index.reject { |_, i| i == target[1] }.map(&:first)
      [d, isolated, aa, af, rest]
    else raise "bad target"
    end
  end

  def consume_pair(state, first, second)
    d, isolated, aa, af, special = state
    drops = []
    [first, second].each do |target|
      case target[0]
      when :isolated then isolated -= 1
      when :aa       then aa -= 1
      when :af       then af -= 1
      when :special  then drops << target[1]
      else raise "bad target"
      end
    end
    rest = special.each_with_index.reject { |_, i| drops.include?(i) }.map(&:first)
    [d, isolated, aa, af, rest]
  end

  def add_fragment(state, fragment, final)
    d, isolated, aa, af, special = state
    fragment = fragment.sort
    case fragment
    when ["A", "A"] then [d, isolated, aa + 1, af, special]
    when ["A", "F"] then [d, isolated, aa, af + 1, special]
    when ["F", "F"]
      final ? [d, isolated, aa, af, special + [fragment]] : nil
    else
      [d, isolated, aa, af, (special + [fragment]).sort]
    end
  end

  def targets(state)
    _, isolated, aa, af, special = state
    result = []
    result << [[:isolated], isolated] if isolated.positive?
    result << [[:aa], 2 * aa] if aa.positive?
    result << [[:af], af] if af.positive?
    special.each_with_index do |fragment, i|
      result << [[:special, i], 1] if fragment.include?("A")
    end
    result
  end

  def valid?(state)
    state[0] <= @k - 1 && frozen_ends(state) <= 2
  end

  def add(hash, state, coefficient)
    hash[state] = (hash[state] || 0) + coefficient
  end

  def step(vector, t, n)
    # P1 (wake/open): relabel exactly the vertex whose future neighbourhood
    # becomes universal at this step.  The forest itself does not change.
    awakened = {}
    vector.each do |state, coefficient|
      d, isolated, aa, af, special = state
      if t <= n + 1
        add(awakened, [d, isolated + 1, aa, af, special], coefficient)
      else
        s = t - n - 1
        if special.include?(["S#{s}"])
          add(awakened,
              [d, isolated + 1, aa, af, special - [["S#{s}"]]], coefficient)
        elsif (i = special.index { |fragment| fragment.include?("C#{s}") })
          fragment = (special[i] - ["C#{s}"] + ["A"]).sort
          rest = special.each_with_index.reject { |_, j| j == i }.map(&:first)
          updated = add_fragment([d, isolated, aa, af, rest], fragment, false)
          add(awakened, updated, coefficient) if updated
        else
          add(awakened, state, coefficient)
        end
      end
    end

    # P2 (insert): the new vertex can use exactly 0, 1, or 2 awake slots.
    capsule = t <= @k - 2
    final = t == n + @k - 1
    output = {}
    awakened.each do |state, coefficient|
      d = state[0]
      available = targets(state)

      # P3 (two edges): classes i<j give a product; within one class the
      # displayed binomial factors choose two distinct components.  Two ends
      # of one AA component are deliberately absent, since they make a cycle.
      available.each_with_index do |(first, m1), i|
        available.each_with_index do |(second, m2), j|
          next if j < i
          multiplicity = if i == j
                           case first[0]
                           when :isolated then m1 * (m1 - 1) / 2
                           when :aa then state[2] >= 2 ? 2 * state[2] * (state[2] - 1) : 0
                           when :af then m1 * (m1 - 1) / 2
                           else 0
                           end
                         else
                           m1 * m2
                         end
          next if multiplicity.zero?
          reduced = consume_pair(state, first, second)
          fragment = [contribution(state, first), contribution(state, second)]
          updated = add_fragment(reduced, fragment, final)
          add(output, updated, coefficient * multiplicity) if updated && valid?(updated)
        end
      end

      # P4 (one edge): one old slot is consumed and the unused new endpoint
      # is a labelled capsule or a permanently frozen endpoint.
      new_token = capsule ? "C#{t}" : "F"
      available.each do |target, multiplicity|
        reduced = consume(state, target)
        reduced = [reduced[0] + 1, reduced[1], reduced[2], reduced[3], reduced[4]]
        updated = add_fragment(reduced,
                               [contribution(state, target), new_token], final)
        add(output, updated, coefficient * multiplicity) if updated && valid?(updated)
      end

      # P5 (zero edges): only a capsule may remain isolated; a non-capsule is
      # frozen immediately and could never enter a Hamilton path.
      if capsule
        updated = [d + 2, state[1], state[2], state[3],
                   (state[4] + [["S#{t}"]]).sort]
        add(output, updated, coefficient) if valid?(updated)
      end
    end
    output
  end

  def head_vector
    vector = { [0, 0, 0, 0, []] => 1 }
    (1..(@k - 2)).each { |t| vector = step(vector, t, BIG) }
    vector
  end

  def reachable
    # P6: least forward-closed bulk state set containing the head support.
    seen = {}
    queue = head_vector.keys
    queue.each { |state| seen[state] = true }
    until queue.empty?
      next_vector = step(queue.to_h { |state| [state, 1] }, @k, BIG)
      queue = next_vector.keys.reject { |state| seen[state] }
      queue.each { |state| seen[state] = true }
    end
    seen.keys
  end

  def accepting?(state)
    # P7: no isolated vertex and exactly one (F,F) component = one spanning
    # unoriented path after every capsule has been opened.
    _, isolated, aa, af, special = state
    isolated.zero? && aa + af + special.size == 1 &&
      special.all? { |fragment| fragment == ["F", "F"] }
  end

  def alive(reachable)
    # Exact co-reachability, not a heuristic pruning rule.  First mark states
    # accepted by the fixed tail, then take the reverse bulk closure.
    successors = reachable.to_h do |state|
      [state, step({ state => 1 }, @k, BIG).keys]
    end
    result = {}
    reachable.each do |state|
      vector = { state => 1 }
      (1..(@k - 2)).each { |j| vector = step(vector, BIG + 1 + j, BIG) }
      result[state] = true if vector.any? { |s, c| accepting?(s) && c.positive? }
    end
    loop do
      grew = false
      reachable.each do |state|
        next if result[state]
        if successors[state].any? { |target| result[target] }
          result[state] = true
          grew = true
        end
      end
      break unless grew
    end
    result
  end

  def write(path)
    full = reachable
    live = alive(full)
    states = full.select { |state| live[state] }.sort_by(&:inspect)
    index = states.each_with_index.to_h
    warn "179962_06_machine: k=#{@k}, reachable=#{full.size}, live=#{states.size}"

    File.open(path, "w") do |stream|
      stream.puts "k #{@k}"
      head = head_vector.select { |state, _| live[state] }
      stream.puts "head #{head.size}"
      head.each { |state, coefficient| stream.puts "#{index[state]} #{coefficient}" }

      edges = []
      states.each do |state|
        step({ state => 1 }, @k, BIG).each do |target, multiplicity|
          edges << [index[state], index[target], multiplicity] if live[target]
        end
      end
      stream.puts "bulk #{states.size} #{edges.size}"
      edges.each { |from, to, multiplicity| stream.puts "#{from} #{to} #{multiplicity}" }

      stream.puts "tail #{@k - 2}"
      current_states = states
      current_index = index
      (1..(@k - 2)).each do |j|
        next_index = {}
        next_states = []
        layer_edges = []
        current_states.each do |state|
          step({ state => 1 }, BIG + 1 + j, BIG).each do |target, multiplicity|
            unless next_index.key?(target)
              next_index[target] = next_states.size
              next_states << target
            end
            layer_edges << [current_index[state], next_index[target], multiplicity]
          end
        end
        stream.puts "layer #{current_states.size} #{next_states.size} #{layer_edges.size}"
        layer_edges.each do |from, to, multiplicity|
          stream.puts "#{from} #{to} #{multiplicity}"
        end
        current_states = next_states
        current_index = next_index
      end

      accepted = current_states.each_with_index.filter_map do |state, i|
        i if accepting?(state)
      end
      stream.puts "accept #{accepted.size}"
      accepted.each { |i| stream.puts i }
    end
    warn "179962_06_machine: wrote #{path}"
  end
end

# An independent small-instance checker.  ExplicitForest keeps labelled
# vertices and actual edge sets; it knows nothing about the aggregated
# transition multiplicities above.  Its state_of method applies only the
# mathematical state definition.  Equality of complete state histograms at
# every layer therefore checks P1--P5, not merely the final sequence value.
module ExplicitForest
  module_function

  def components(vertex_count, edges)
    adjacency = Array.new(vertex_count + 1) { [] }
    edges.each do |u, v|
      adjacency[u] << v
      adjacency[v] << u
    end
    seen = Array.new(vertex_count + 1, false)
    result = []
    (1..vertex_count).each do |root|
      next if seen[root]
      stack = [root]
      seen[root] = true
      component = []
      until stack.empty?
        u = stack.pop
        component << u
        adjacency[u].each do |v|
          next if seen[v]
          seen[v] = true
          stack << v
        end
      end
      result << component
    end
    [result, adjacency]
  end

  def endpoint_type(vertex, t, n, k)
    return "A" if vertex <= t
    return nil if vertex <= n + 1 # sleeping low vertex; necessarily isolated
    s = vertex - n - 1
    return "C#{s}" if s <= k - 2 && vertex > t
    return "A" if s <= k - 2 # opened capsule
    "F"
  end

  def state_of(edges, t, n, k)
    vertex_count = n + 1 + t
    comps, adjacency = components(vertex_count, edges)
    isolated = 0
    aa = 0
    af = 0
    special = []
    comps.each do |component|
      if component.length == 1
        vertex = component[0]
        type = endpoint_type(vertex, t, n, k)
        if type == "A"
          isolated += 1
        elsif type&.start_with?("C")
          special << ["S#{type[1..]}"]
        elsif type == "F"
          return nil # frozen isolated vertex
        end
        next
      end
      endpoints = component.select { |vertex| adjacency[vertex].length == 1 }
      raise "explicit graph is not a path" unless endpoints.length == 2
      fragment = endpoints.map { |vertex| endpoint_type(vertex, t, n, k) }.sort
      raise "sleeping endpoint acquired an edge" if fragment.any?(&:nil?)
      case fragment
      when ["A", "A"] then aa += 1
      when ["A", "F"] then af += 1
      when ["F", "F"]
        return nil unless t == n + k - 1
        special << fragment
      else
        special << fragment
      end
    end
    state = [2 * t - edges.length, isolated, aa, af, special.sort]
    frozen = af + special.sum { |fragment| fragment.count("F") }
    return nil if state[0] > k - 1 || frozen > 2
    state
  end

  def same_component?(first, second, vertex_count, edges)
    comps, = components(vertex_count, edges)
    comps.any? { |component| component.include?(first) && component.include?(second) }
  end

  def next_forests(forests, t, n, k)
    result = {}
    forests.each_key do |edges|
      vertex_count = n + t
      _, adjacency = components(vertex_count, edges)
      eligible = (1..t).select { |vertex| adjacency[vertex].length < 2 }
      choices = [[]] + eligible.map { |u| [u] }
      eligible.combination(2) do |u, v|
        choices << [u, v] unless same_component?(u, v, vertex_count, edges)
      end
      new_vertex = n + 1 + t
      choices.each do |targets|
        added = targets.map { |u| [u, new_vertex] }
        candidate = (edges + added).map(&:sort).sort.freeze
        result[candidate] = true if state_of(candidate, t, n, k)
      end
    end
    result
  end

  def histograms(n, k)
    forests = { [].freeze => true }
    result = []
    (1..(n + k - 1)).each do |t|
      forests = next_forests(forests, t, n, k)
      histogram = Hash.new(0)
      forests.each_key { |edges| histogram[state_of(edges, t, n, k)] += 1 }
      result << histogram
    end
    result
  end

  def brute_permutations(n, k)
    size = 2 * n + k
    (1..size).to_a.permutation.count do |permutation|
      permutation.each_cons(2).all? { |u, v| (u - v).abs > n }
    end
  end
end

def run_self_check
  # These include capsule creation, bulk steps, capsule opening and the final
  # (F,F) exception.  The largest brute-force case has 10! permutations.
  cases = [[3, 2], [3, 3], [4, 2], [4, 3]]
  cases.each do |k, n|
    machine = InsertionMachine.new(k)
    aggregate = { [0, 0, 0, 0, []] => 1 }
    explicit = ExplicitForest.histograms(n, k)
    (1..(n + k - 1)).each do |t|
      aggregate = machine.step(aggregate, t, n)
      unless aggregate == explicit[t - 1]
        missing = explicit[t - 1].reject { |state, count| aggregate[state] == count }
        extra = aggregate.reject { |state, count| explicit[t - 1][state] == count }
        abort "self-check state mismatch k=#{k}, n=#{n}, t=#{t}: " \
              "missing=#{missing.first.inspect}, extra=#{extra.first.inspect}"
      end
    end
    unoriented = aggregate.sum do |state, count|
      machine.accepting?(state) ? count : 0
    end
    oriented = 2 * unoriented
    brute = ExplicitForest.brute_permutations(n, k)
    abort "self-check count mismatch k=#{k}, n=#{n}" unless oriented == brute
    warn "self-check ok: k=#{k}, n=#{n}, value=#{oriented}, " \
         "#{n + k - 1} layer histograms"
  end
  warn "self-check ok: proof branches P1--P7 and definition counts"
end

if ARGV == ["--self-check"]
  run_self_check
  exit 0
end

unless ARGV.length.between?(1, 2) && ARGV[0].match?(/\A\d+\z/)
  warn "usage: ruby 179962_06_machine.rb K [OUTPUT.machine]"
  warn "       ruby 179962_06_machine.rb --self-check"
  exit 1
end
k = ARGV[0].to_i
abort "error: K must be at least 3" if k < 3
output = ARGV[1] || "179962_06_k#{k}.machine"
InsertionMachine.new(k).write(output)
