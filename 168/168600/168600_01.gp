M=12;

\\ a(0) = 1; a(n) = 2^(n-1) * (n-1)! * Sum_{i, j, k>=0 and i+j+k=n-1} (n-i)/2^i * a(i) * a(j) * a(k)/(i! * j! * k!).
a(n) = if(n==0, 1, 2^(n-1) * (n-1)! * sum(i=0, n-1, sum(j=0, n-1-i, (n-i)/2^i * a(i) * a(j) * a(n-1-i-j)/(i! * j! * (n-1-i-j)!) )));
for(n=0, M, print1(a(n),", "));