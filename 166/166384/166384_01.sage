n = 17

# a(n) = RisingFactorial(2*n+1,3*n).
def a(n): return rising_factorial(2 * n + 1, 3 * n)
print([a(n) for n in range(n + 1)])