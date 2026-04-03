\\ G.f. A(x) satisfies A(x) = 1 + x*A(x)*(1 + A(x)) + x^2*d/dx A(x).
my(A=1, N=30); for(k=1, N, A=1 + x*A*(1 + A) + x^2*deriv(A) + O(x^N)); Vec(A)
