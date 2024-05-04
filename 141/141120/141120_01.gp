\\ Define the sequence b(n,m) as follows. If n<m, b(n,m) = 0, else if n=m, b(n,m) = 1, otherwise b(n,m) = 1/5 * ( 25^(n-m) * binomial(m,n-m) - Sum_{l=m+1..n-1} (b(n,l) + Sum_{k=l..n} (b(n,k) + Sum_{j=k..n} (b(n,j) + Sum_{i=j..n} b(n,i) * b(i,j)) * b(j,k)) * b(k,l)) * b(l,m) ). a(n) = b(n,1).
b(n, m) = if(n<m, 0, if(n==m, 1, 1/5 * (25^(n-m) * binomial(m, n-m) - sum(l=m+1, n-1, (b(n, l) + sum(k=l, n, (b(n, k) + sum(j=k, n, (b(n, j) + sum(i=j, n, b(n, i) * b(i, j))) * b(j, k))) * b(k, l))) * b(l, m)))));
a(n) = b(n, 1);
for(n=1, 10, print1(a(n), ", "))

