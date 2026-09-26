\\ a(0) = a(1) = 1; a(n) = (1/2) * (4^(n-1)*a(n-1) - Sum_{k=2..n-1} a(k) * a(n+1-k)). 
print("NG");
a(n) = if(n<1, 1, (1/2) * (4^(n-1)*a(n-1) - sum(k=2, n-1, a(k) * a(n+1-k))));
for(n=0, 11, print1(a(n),", "));

print("OK");
a(n) = if(n<2, 1, (1/2) * (4^(n-1)*a(n-1) - sum(k=2, n-1, a(k) * a(n+1-k))));
for(n=0, 11, print1(a(n),", "));