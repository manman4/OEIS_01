M=7;

row(n) = Vecrev(prod(k=1, n, k!-x));
for(n=0, M, print1(row(n)); print); 