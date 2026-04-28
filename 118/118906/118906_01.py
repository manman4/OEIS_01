# a(0) = 1; a(n) = |a(n-1) - n| if n is not one of the earlier terms of the sequence, otherwise a(n) = a(n-1) + n.
def A(n):
  arr = [1]
  for i in range(1, n + 1):
    if i in arr:
      arr.append(arr[-1] + i)
    else:
      arr.append(abs(arr[-1] - i))

  return arr

n = 10000
arr = A(n)
for i in range(0, n + 1):
  print(i, arr[i])