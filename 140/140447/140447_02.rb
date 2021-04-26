require 'prime'
ary = []
Prime.each(487447).each{|i| ary += [i, i + 22] if (i + 22).prime?}
p ary
