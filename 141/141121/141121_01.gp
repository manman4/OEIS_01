\\ Define the sequence b(n,m) as follows. 
\\ If n<m, b(n,m) = 0, else if n=m, b(n,m) = 1, 
\\ otherwise b(n,m) = 1/6 * ( 36^(n-m) * binomial(m,n-m) - Sum_{l=m+1..n-1} (b(n,l) + Sum_{k=l..n} (b(n,k) + Sum_{j=k..n} (b(n,j) + Sum_{i=j..n} (b(n,i) + Sum_{h=i..n} b(n,h) * b(h,i)) * b(i,j)) * b(j,k)) * b(k,l)) * b(l,m) ). 
\\ a(n) = b(n,1)

b(n,m) = if(n<m, 0, if(n==m, 1, 1/6 * (36^(n-m) * binomial(m,n-m) - sum(l=m+1, n-1, (b(n,l) + sum(k=l, n, (b(n,k) + sum(j=k, n, (b(n,j) + sum(i=j, n, (b(n,i) + sum(h=i, n, b(n,h) * b(h,i))) * b(i,j))) * b(j,k))) * b(k,l))) * b(l,m)))));
a(n) = b(n, 1);
for(n=1, 8, print1(a(n), ", "))

a=[1, 6, -180, 8640, -498960, 31434480, -2055943296, 135216506304]
f(x) = sum(n=1, #a, a[n]*x^n);

b=[1, 3, -18, 189, -2430, 34020, -486972, 6786261, -86946372, 919825956];
g(x) = sum(n=1, #b, b[n]*x^n);
Vec(g(4*x)/4 + x*O(x^#b))
Vec(f(f(x)) + x*O(x^#a))
c=[1, 2, -4, 16, -80, 432, -2304, 10944, -35328, -74112, 2736384, -30853632];
h(x) = sum(n=1, #c, c[n]*x^n);
Vec(h(9*x)/9 + x*O(x^#c))
Vec(f(f(f(x))) + x*O(x^#a))

