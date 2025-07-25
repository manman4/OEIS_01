M=18;


\\ a(0) = 1; a(n) = Sum_{k=0..n-1} (1 + k) * (1 + k/2) * binomial(n-1,k) * a(k) * a(n-1-k).
a(n) = if(n==0, 1, sum(k=0, n-1, (1 + k) * (1 + k/2) * binomial(n-1,k) * a(k) * a(n-1-k)));
for(n=0, 12, print1(a(n), ", "));

a_vector(n) = my(v=vector(n+1)); v[1]=1; for(i=1, n, v[i+1]=sum(j=0, i-1, (1+j)*(1+j/2)*binomial(i-1, j)*v[j+1]*v[i-j])); v;
for(n=0, M, print(a_vector(n)));