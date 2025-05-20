M=13;

\\ a(n) = (2*n)! * [x^(2*n)] (-log(1 - x))^n / n!.
a(n) = my(x='x+O('x^(2*n+1))); (2*n)! * polcoef( (-log(1 - x))^n / n!, 2*n);
for(n=0, M, print1(a(n),", "));
