M=10000;

a008441(n) = sumdiv(4*n+1, d, (-1)^(d\2));
a(n) = if(n%2==0, a008441(n/2), if(n%6==1, a008441((n-1)/6)));
for(n=0, M, write("/Users/xxx/Desktop/b125079_1.txt", n, " ", a(n)))