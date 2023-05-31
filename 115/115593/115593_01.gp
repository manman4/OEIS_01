a(n) = if(n==0, 1, (1/n) * sum(k=1, n, sumdiv(k, d, (d%2==1) *d * a(d\2) ) * a(n-k)));
for(n=0, 20, print1(a(n),", "))

