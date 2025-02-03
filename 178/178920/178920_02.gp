\\ E.g.f. A(x) satisfies A(x) = x * (1 + A(x)) * exp( A(x) * (1 + A(x)) ).
seq(n) = my(A=1); for(i=1, n, A=x * (1 + A) * exp( A * (1 + A) +x*O(x^n) ) ); Vec(serlaplace(A));
seq(19)                   
