M=21;

a(n) = my(A=x); for(i=1, n, A = x * exp( A * exp(A +x*O(x^n)) + A^2 * exp(2*A +x*O(x^n)) ) ); n!*polcoeff(A, n);
for(n=1, M, print1(a(n), ", "))