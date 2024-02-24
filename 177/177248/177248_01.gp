T(n, k) = my(x='x+O('x^(n+2))); polcoef((1/k!) * sum(j=k, n, j! * x^(j+k) / (1+x^2)^(j+1)), n);
for(n=0, 12, for(k=0, n\2, print1(T(n, k),", ")))