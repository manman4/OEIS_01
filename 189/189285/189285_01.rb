#!/usr/bin/env ruby
# frozen_string_literal: true

# A189285 and directed a_{d,d} -- Ruby port of 189285_03.c.
#
# Count permutations p of {1,...,n} satisfying
#
#                 p(i+d) - p(i) != d,   1 <= i <= n-d.
#
# This evaluates the Spahn--Zeilberger tiling-partition formula
#
#   a_{d,d}(n) = sum_H (-1)^r W(H)^2
#                  * (n-r-c)! * product_l h_l!,
#
# where h_l counts tiles of size l+1, r=sum_l l*h_l,
# c=sum_l h_l, and W(H) is the number of d-step tilings with histogram H.
# It is the same algorithm as 189285_03.c; Ruby Integer replaces GMP and
# Ruby Hash replaces the C open-addressed histogram table.
#
# A histogram is packed into one Ruby Integer using seven bits per count.
# Because n<=128, the total number of non-singleton tiles is at most 64, so
# no field can overflow seven bits.  Componentwise histogram addition is
# consequently one exact Integer addition without inter-field carries.
#
# Reference:
#   G. Spahn and D. Zeilberger,
#   "Counting Permutations Where The Difference Between Entries Located
#    r Places Apart Can never be s",
#   Enumerative Combinatorics and Applications 3:2 (2023), Article S2R10,
#   https://doi.org/10.54550/ECA2023V3S2R10
#
# Usage:
#   ruby 189285_01.rb
#   ruby 189285_01.rb 40
#   ruby 189285_01.rb --term 40
#   ruby 189285_01.rb --upto 40
#   ruby 189285_01.rb --d 2 --term 40
#   ruby 189285_01.rb --check
#
# A range run does not create a b-file.  It prints a(0)..a(MAX_N) to
# standard output as one comma-separated line.

module A189285Tiling
  A189285_D = 6
  SEQUENCE_OFFSET = 0
  DEFAULT_MAX_N = 20
  DEFAULT_CHECK_N = 24
  MAX_SUPPORTED_N = 128
  MAX_SUPPORTED_D = 64
  BRUTE_CHECK_MAX_N = 16
  GENERAL_CHECK_MAX_D = 8
  GENERAL_CHECK_MAX_N = 12
  HISTOGRAM_BITS = 7
  HISTOGRAM_MASK = (1 << HISTOGRAM_BITS) - 1

  class CalculationError < StandardError; end

  KNOWN_SEQUENCES = {
    2 => [
      'A189281',
      %w[
        1 1 2 5 18 75 410 2729 20906 181499 1763490 18943701
        222822578 2847624899 39282739034 581701775369 9202313110506
        154873904848803 2762800622799362 52071171437696453
        1033855049655584786 21567640717569135515
        471630531427793184474 10787660036599729160073
        257590656485400508526570
      ].map!(&:to_i).freeze
    ].freeze,
    3 => [
      'A189282',
      %w[
        1 1 2 6 22 98 534 3414 25498 217338 2080990 22076030
        256888218 3252308706 44497313158 654139144158 10281397705242
        172033123244330 3052895403376110 57266799403366334
        1132124282036449570 23524895818926592242
        512577206181190933062 11685351024627411070118
        278171270561272398036690
      ].map!(&:to_i).freeze
    ].freeze,
    4 => [
      'A189283',
      %w[
        1 1 2 6 24 114 628 4062 30360 255186 2414292 25350954
        292378968 3673917102 49928069188 729534877758 11403682481112
        189862332575658 3354017704180052 62654508729565554
        1233924707891272728 25550498290562247438
        554913370184289495780 12612648556263898345758
        299411750583810718488216
      ].map!(&:to_i).freeze
    ].freeze,
    5 => [
      'A189284',
      %w[
        1 1 2 6 24 120 696 4572 34260 290328 2751480 28686024
        328764732 4106158164 55495145304 806797105320 12554890849992
        208164423163908 3663256621120548 68188490015132040
        1338490745511631080 27630826605742438968
        598383049497841208340 13564486434215852731596
        321206759087756505407112
      ].map!(&:to_i).freeze
    ].freeze,
    6 => [
      'A189285',
      %w[
        1 1 2 6 24 120 720 4920 37488 319644 3033264 31784280
        364902480 4538652840 61102571376 885045657564 13722397569072
        226742901078120 3977354871110160 73816786920489720
        1444940702597713008 29750236302549282948
        642693417769786746864 14535097378893765189240
        343437047642562028552080
      ].map!(&:to_i).freeze
    ].freeze
  }.freeze

  module_function

  def parse_integer(text, label, minimum, maximum)
    value = Integer(text, 10)
    unless value.between?(minimum, maximum)
      raise CalculationError,
            "#{label} must be in #{minimum}..#{maximum}: #{text}"
    end
    value
  rescue ArgumentError
    raise CalculationError,
          "#{label} must be in #{minimum}..#{maximum}: #{text}"
  end

  def parse_n(text, label)
    parse_integer(text, label, SEQUENCE_OFFSET, MAX_SUPPORTED_N)
  end

  def parse_d(text)
    parse_integer(text, 'D', 2, MAX_SUPPORTED_D)
  end

  def binomial(n, k)
    return 0 if k.negative? || k > n

    k = [k, n - k].min
    value = 1
    1.upto(k) do |i|
      value = value * (n - k + i) / i
    end
    value
  end

  def generate_local_recursive(path_size, length_limit, length_index,
                               edge_count, component_count,
                               component_orders, packed_histogram, output)
    if length_index == length_limit
      gap_choices = binomial(path_size - edge_count, component_count)
      output << [packed_histogram, gap_choices * component_orders]
      return
    end

    length = length_index + 1
    remaining_vertices = path_size - edge_count - component_count
    maximum = remaining_vertices / (length + 1)
    0.upto(maximum) do |count|
      next_histogram =
        packed_histogram | (count << (HISTOGRAM_BITS * length_index))
      next_orders = component_orders *
                    binomial(component_count + count, count)
      generate_local_recursive(
        path_size, length_limit, length_index + 1,
        edge_count + length * count, component_count + count,
        next_orders, next_histogram, output
      )
    end
  end

  def generate_local_patterns(path_size)
    unless path_size.between?(1, MAX_SUPPORTED_N / 2)
      raise CalculationError, 'residue path exceeds the supported limit'
    end

    output = []
    length_limit = path_size - 1
    generate_local_recursive(
      path_size, length_limit, 0, 0, 0, 1, 0, output
    )
    output.freeze
  end

  def build_tiling_coefficients(path_sizes)
    current = { 0 => 1 }
    cache = {}
    peak_histogram_count = 1
    convolution_pairs = 0

    path_sizes.each do |path_size|
      patterns = cache[path_size] ||= generate_local_patterns(path_size)
      following = Hash.new(0)
      current.each do |histogram, coefficient|
        patterns.each do |local_histogram, local_coefficient|
          # Seven-bit fields cannot carry: at most floor(n/2)<=64
          # non-singleton tiles occur in the complete histogram.
          combined = histogram + local_histogram
          following[combined] += coefficient * local_coefficient
          convolution_pairs += 1
        end
      end
      current = following
      peak_histogram_count = [peak_histogram_count, current.length].max
    end

    [current, peak_histogram_count, convolution_pairs]
  end

  def factorial_table(n)
    factorial = Array.new(n + 1, 1)
    1.upto(n) { |i| factorial[i] = factorial[i - 1] * i }
    factorial
  end

  def compute_formula(n, d)
    unless n.between?(0, MAX_SUPPORTED_N) && d.between?(2, MAX_SUPPORTED_D)
      raise CalculationError, 'unsupported n or d'
    end

    started = Process.clock_gettime(Process::CLOCK_MONOTONIC)
    factorial = factorial_table(n)
    if n <= d
      seconds = Process.clock_gettime(Process::CLOCK_MONOTONIC) - started
      return [
        factorial[n],
        {
          histogram_count: 1,
          peak_histogram_count: 1,
          convolution_pairs: 0,
          coefficient_sum: 1,
          seconds: seconds
        }
      ]
    end

    path_sizes = 1.upto(d).map { |k| (n - k) / d + 1 }
    length_limit = path_sizes.max - 1
    coefficients, peak, pairs = build_tiling_coefficients(path_sizes)

    coefficient_sum = coefficients.values.sum
    expected_sum = 1 << (n - d)
    unless coefficient_sum == expected_sum
      raise CalculationError,
            'tiling coefficients do not sum to 2^(n-d)'
    end

    result = 0
    coefficients.each do |packed_histogram, coefficient|
      histogram = packed_histogram
      r = 0
      c = 0
      histogram_factorial = 1
      0.upto(length_limit - 1) do |length_index|
        count = histogram & HISTOGRAM_MASK
        unless count.zero?
          r += (length_index + 1) * count
          c += count
          histogram_factorial *= factorial[count] if count > 1
        end
        histogram >>= HISTOGRAM_BITS
      end

      singleton_count = n - r - c
      raise CalculationError, 'invalid tile histogram' if singleton_count.negative?

      term = coefficient * coefficient
      term *= factorial[singleton_count] * histogram_factorial
      result += r.even? ? term : -term
    end

    unless result.between?(0, factorial[n])
      raise CalculationError, 'formula result is outside 0..n!'
    end

    seconds = Process.clock_gettime(Process::CLOCK_MONOTONIC) - started
    [
      result,
      {
        histogram_count: coefficients.length,
        peak_histogram_count: peak,
        convolution_pairs: pairs,
        coefficient_sum: coefficient_sum,
        seconds: seconds
      }
    ]
  end

  def verify_known_value(value, n, d)
    sequence = KNOWN_SEQUENCES[d]
    return if sequence.nil?

    oeis_id, terms = sequence
    return if n >= terms.length

    expected = terms[n]
    return if value == expected

    raise CalculationError,
          "#{oeis_id} mismatch at n=#{n}: got #{value}, expected #{expected}"
  end

  def report_formula(n, d, stats)
    warn format(
      '189285_01.rb: d=%d, n=%d, histograms=%d, peak=%d, ' \
      'convolution pairs=%d, %.3f s',
      d, n, stats[:histogram_count], stats[:peak_histogram_count],
      stats[:convolution_pairs], stats[:seconds]
    )
  end

  def compute_value(n, d, verbose:)
    value, stats = compute_formula(n, d)
    verify_known_value(value, n, d)
    report_formula(n, d, stats) if verbose
    value
  end

  def brute_recursive(n, d, starts_chain, memo, used, assigned, last_value)
    return 1 if assigned == n

    last_value = n if starts_chain[assigned]
    index = used * (n + 1) + last_value
    cached = memo[index]
    return cached unless cached.nil?

    total = 0
    available = ((1 << n) - 1) ^ used
    until available.zero?
      bit = available & -available
      available ^= bit
      value = bit.bit_length - 1
      next if last_value < n && value - last_value == d

      total += brute_recursive(
        n, d, starts_chain, memo, used | bit, assigned + 1, value
      )
    end
    memo[index] = total
  end

  def brute_count(n, d)
    unless n.between?(0, BRUTE_CHECK_MAX_N)
      raise CalculationError,
            'independent check n is outside its supported range'
    end

    starts_chain = []
    0.upto([d, n].min - 1) do |residue|
      first = true
      position = residue
      while position < n
        starts_chain << first
        first = false
        position += d
      end
    end
    unless starts_chain.length == n
      raise CalculationError, 'independent-check position ordering failed'
    end

    memo = Array.new((1 << n) * (n + 1))
    brute_recursive(n, d, starts_chain, memo, 0, 0, n)
  end

  def check_implementation(maximum_n)
    0.upto(maximum_n) do |n|
      value = compute_value(n, A189285_D, verbose: false)
      next if n > BRUTE_CHECK_MAX_N

      independent = brute_count(n, A189285_D)
      unless value == independent
        raise CalculationError,
              "A189285 DP mismatch at n=#{n}: " \
              "formula=#{value}, DP=#{independent}"
      end
    end

    KNOWN_SEQUENCES.each do |d, (oeis_id, terms)|
      terms.each_with_index do |expected, n|
        value, = compute_formula(n, d)
        next if value == expected

        raise CalculationError,
              "#{oeis_id} mismatch at n=#{n}: " \
              "got #{value}, expected #{expected}"
      end
    end

    2.upto(GENERAL_CHECK_MAX_D) do |d|
      0.upto(GENERAL_CHECK_MAX_N) do |n|
        value, = compute_formula(n, d)
        independent = brute_count(n, d)
        next if value == independent

        raise CalculationError,
              "general-d DP mismatch at d=#{d}, n=#{n}: " \
              "formula=#{value}, DP=#{independent}"
      end
    end

    verified = KNOWN_SEQUENCES.map do |d, (oeis_id, terms)|
      "#{oeis_id}(d=#{d},n=0..#{terms.length - 1})"
    end.join(', ')
    brute_maximum = [maximum_n, BRUTE_CHECK_MAX_N].min
    puts "ok: OEIS b-file verified: #{verified}; " \
         "independent DP for d=2..#{GENERAL_CHECK_MAX_D} " \
         "through n=#{GENERAL_CHECK_MAX_N} " \
         "(and d=#{A189285_D} through n=#{brute_maximum})"
    0
  end

  def print_sequence(maximum_n, d)
    values = 0.upto(maximum_n).map do |n|
      compute_value(n, d, verbose: false)
    end
    puts values.join(', ')
    0
  end

  def usage(program)
    warn <<~USAGE
      usage: #{program} [MAX_N]
             #{program} --term N
             #{program} --upto MAX_N
             #{program} --d D [MAX_N]
             #{program} --d D --term N
             #{program} --d D --upto MAX_N
             #{program} --check [MAX_N]

      D defaults to #{A189285_D} (A189285) and may be 2..#{MAX_SUPPORTED_D}.
      MAX_N defaults to #{DEFAULT_MAX_N} and may be at most #{MAX_SUPPORTED_N}.
      A range run prints a(0)..a(MAX_N) as comma-separated values.
      It does not create a b-file.
      --check verifies A189281..A189285 for d=2..6.
    USAGE
  end

  def main(arguments)
    program = File.basename($PROGRAM_NAME)
    if arguments.length == 1 && %w[--help -h].include?(arguments[0])
      usage(program)
      return 0
    end
    if arguments.first == '--check'
      if arguments.length > 2
        usage(program)
        return 1
      end
      maximum_n = arguments.length == 2 ?
        parse_n(arguments[1], 'MAX_N') : DEFAULT_CHECK_N
      return check_implementation(maximum_n)
    end

    mode = :upto
    d = A189285_D
    n = nil
    have_d = false
    argument = 0
    while argument < arguments.length
      text = arguments[argument]
      case text
      when '--d'
        if have_d || argument + 1 >= arguments.length
          usage(program)
          return 1
        end
        argument += 1
        d = parse_d(arguments[argument])
        have_d = true
      when '--term', '--upto'
        if !n.nil? || argument + 1 >= arguments.length
          usage(program)
          return 1
        end
        mode = text == '--term' ? :term : :upto
        argument += 1
        n = parse_n(arguments[argument], mode == :term ? 'N' : 'MAX_N')
      else
        if text.start_with?('-') || !n.nil?
          usage(program)
          return 1
        end
        n = parse_n(text, 'MAX_N')
        mode = :upto
      end
      argument += 1
    end

    n ||= DEFAULT_MAX_N
    if mode == :term
      value = compute_value(n, d, verbose: true)
      puts "#{n} #{value}"
      0
    else
      print_sequence(n, d)
    end
  end
end

begin
  exit A189285Tiling.main(ARGV)
rescue A189285Tiling::CalculationError, SystemCallError => error
  warn "error: #{error.message}"
  exit 1
end
