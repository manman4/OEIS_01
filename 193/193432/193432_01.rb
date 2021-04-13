# def A(k, n)
#   a, b = (0..n).map {|x| x ** 2 + k}, [1] * (n + 1)
#     1.upto(n) do |x|
#       next if (q = a[x]) == 1
#       x.step(n, q) do |y|
#         cnt = 0
#         while a[y] % q == 0
# 	        a[y] /= q
#           cnt += 1
#         end
#         b[y] *= cnt + 1
#       end
#     end
#   b[1..-1]
# end

require 'prime'
def A193432(n)
  (0..n).map{|i| (i * i + 1).prime_division.inject(1){|s, j| s * (j[1] + 1)}}
end

n = 100
ary = A193432(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
