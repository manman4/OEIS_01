default(parisize, 12000000000)

M=10000;

my(x='x+O('x^(M+10))); v=prod(k=1, M, (1 + x^(2*k-1))^moebius(2*k-1));
for(n=0, M, write("/Users/xxx/Desktop/b117209_1.txt", n, " ", polcoef(v, n)))
