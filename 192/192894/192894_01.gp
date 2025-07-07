M=20;

\\ G.f. A(x) satisfies A(x) = 1/( 1 - x*(A(x)*A(-x))^6 ).
my(A=1, n=20); for(i=1, n, A= 1/( 1 - x*(A*subst(A, x, -x))^6 + x*O(x^n) )); Vec(A)

\\ G.f. A(x) satisfies A(x)*A(-x) = (A(x) + A(-x))/2
my(A=1, n=20); for(i=1, n, A= 1/( 1 - x*(A*subst(A, x, -x))^6 + x*O(x^n) )); Vec( A * subst(A, x, -x) - (A + subst(A, x, -x))/2 )




