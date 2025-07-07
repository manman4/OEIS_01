M=20;

\\ a(0) = 1; a(n) = Sum_{k=0..floor((n-1)/3)} a(3*k) * a(n-1-3*k).
a(n) = if(n==0, 1, sum(k=0, (n-1)\3, a(3*k) * a(n-1-3*k)));
for(n=0, M, print1(a(n),", "));