\\ a(n) = 1 + (n-1)/2 * Sum_{k=0..n-1} a(k) * a(n-1-k).
a(n) = 1 + (n-1)/2 * sum(k=0, n-1, a(k) * a(n-1-k));
for(n=0, 15, print1(a(n),", "))

a_vector(n) = my(v=vector(n+1)); for(i=0, n, v[i+1]=1+(n-1)/2*sum(j=0, i-1, v[j+1]*v[i-j])); v;
a_vector(22)
