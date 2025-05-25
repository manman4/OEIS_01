M=16;

\\ a(n) = (3*n)! * [x^(3*n)] 1/(1 - x)^(n+1).
a(n) = my(x='x+O('x^(3*n+1))); (3*n)! * polcoef( 1/(1 - x)^(n+1), 3*n);
for(n=0, M, print1(a(n),", "));
