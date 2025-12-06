\\ Recurrence: a(n) = (n+2)*a(n-1) - (n-2)*a(n-2) for n > 2.
a(n) = my(v=[1,1,5]); if(n<3, v[n+1], (n+2)*a(n-1)-(n-2)*a(n-2));
for(n=0, 15, print1(a(n),", "));

\\ 以下はダメ
a(n) = my(v=[1,1]); if(n<2, v[n+1], (n+2)*a(n-1)-(n-2)*a(n-2));
for(n=0, 15, print1(a(n),", "));

a001340(n) = 2*n!*polcoef(exp(x+x*O(x^n))/(1-x)^3, n);
for(n=0, 15, print1(a001340(n),", "));
\\ a(n) = A001340(n-1) - (n^2-n+1)*(n-1)! for n > 0. 
a(n) = if(n==0, 1, a001340(n-1)-(n^2-n+1)*(n-1)!);
for(n=0, 15, print1(a(n),", "));