M=7;

\\ a(0) = 1; a(n) = (-1)^(n-1) * Sum_{i, j, k, l, m>=0 and i+j+k+l+m=n-1} a(i) * a(j) * a(k) * a(l) * a(m).
a(n) = if(n==0, 1, (-1)^(n-1) * sum(i=0, n-1, sum(j=0, n-1-i, sum(k=0, n-1-i-j, sum(l=0, n-1-i-j-k, a(i) * a(j) * a(k) * a(l) * a(n-1-i-j-k-l) )))));
for(n=0, M, print1(a(n),", "));