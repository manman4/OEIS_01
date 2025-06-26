M=22;

T(n, k) = my(x='x+O('x^(n+1))); n!*polcoef(asin(x)^k/k!, n);
\\ a(n) = Sum_{k=0..n} k! * A385343(n,k). 
a(n) = sum(k=0, n, k! * T(n, k));
for(n=0, M, print1(a(n),", "));