\\ G.f. A(x) satisfies A(x) = 1 - x*d/dx log(1 - x*(1 + A(x))).
my(A=1, N=30); for(k=1, N, A=1 - x*deriv(log(1 - x*(1 + A))) + x*O(x^N)); Vec(A)
