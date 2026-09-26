\\ a(0) = a(1) = 1; a(n) = (1/3) * (9^(n-1)*a(n-1) - Sum_{1<=i,j,k<=n-1 and i+j+k=n+2} a(i) * a(j) * a(k)).
print("NG");
a(n) = if(n<1, 1, (1/3) * (9^(n-1)*a(n-1) - sum(i=1, n-1, sum(j=1, n-1, sum(k=1, n-1, if(i+j+k==n+2, a(i) * a(j) * a(k), 0))))));
for(n=0, 8, print1(a(n),", "));

print("OK");
a(n) = if(n<2, 1, (1/3) * (9^(n-1)*a(n-1) - sum(i=1, n-1, sum(j=1, n-1, sum(k=1, n-1, if(i+j+k==n+2, a(i) * a(j) * a(k), 0))))));
for(n=0, 8, print1(a(n),", "));