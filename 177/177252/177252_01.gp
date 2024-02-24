\\ G.f. of column k: (1/k!) * Sum_{j>=k} j! * x^(j+3*k) / (1+x^4)^(j+1).
T(n, k) = my(x='x+O('x^(n+2))); polcoef((1/k!) * sum(j=k, n, j! * x^(j+3*k) / (1+x^4)^(j+1)), n);
for(n=0, 16, for(k=0, n\4, print1(T(n, k),", ")))