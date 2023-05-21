T(n, r) = my(x='x+O('x^(n+10))); polcoef(1/prod(k=1, n, 1-x^k)*sum(k=1, n, (-1)^(k-1) * x^(r*k) * ( x^(k*(3*k-1)/2) - x^(k*(3*k+1)/2) ) ), n);
for(n=1, 14, for(r=0, n-1, print1(T(n, r), ", ")))
