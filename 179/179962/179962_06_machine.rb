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

    capsule = t <= @k - 2
    final = t == n + @k - 1
    output = {}
    awakened.each do |state, coefficient|
      d = state[0]
      available = targets(state)

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

      new_token = capsule ? "C#{t}" : "F"
      available.each do |target, multiplicity|
        reduced = consume(state, target)
        reduced = [reduced[0] + 1, reduced[1], reduced[2], reduced[3], reduced[4]]
        updated = add_fragment(reduced,
                               [contribution(state, target), new_token], final)
        add(output, updated, coefficient * multiplicity) if updated && valid?(updated)
      end

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
    _, isolated, aa, af, special = state
    isolated.zero? && aa + af + special.size == 1 &&
      special.all? { |fragment| fragment == ["F", "F"] }
  end

  def alive(reachable)
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

unless ARGV.length.between?(1, 2) && ARGV[0].match?(/\A\d+\z/)
  warn "usage: ruby 179962_06_machine.rb K [OUTPUT.machine]"
  exit 1
end
k = ARGV[0].to_i
abort "error: K must be at least 3" if k < 3
output = ARGV[1] || "179962_06_k#{k}.machine"
InsertionMachine.new(k).write(output)
