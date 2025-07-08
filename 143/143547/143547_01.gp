\\ G.f. A(x) satisfies A(x) = 1/( 1 - x*(A(x)*A(-x))^3 ).
my(A=1, n=22); for(i=1, n, A= 1/( 1 - x*(A*subst(A, x, -x))^3 + x*O(x^n) )); Vec(A)

\\ G.f. A(x) satisfies A(x)*A(-x) = (A(x) + A(-x))/2
my(A=1, n=22); for(i=1, n, A= 1/( 1 - x*(A*subst(A, x, -x))^3 + x*O(x^n) )); Vec( A * subst(A, x, -x) - (A + subst(A, x, -x))/2 )
my(A=1, n=22); for(i=1, n, A= 1/( 1 - x*(A*subst(A, x, -x))^3 + x*O(x^n) )); Vec( A * subst(A, x, -x) )

\\ A(x)*A(-x) = (A(x) + A(-x))/2 = G(x^2), where G(x) = 1 + x*G(x)^7.
my(G=1, n=22); for(i=1, n, G= 1 + x*(G^7) + x*O(x^n) ); Vec(subst(G, x, x^2))



