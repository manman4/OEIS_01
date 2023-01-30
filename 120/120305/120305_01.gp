M=11;

a(n) = sum(i=0, 2*n, (-1)^i*i!*polcoef(sum(j=0, n, x^j/j!)^2, i)); 
for(n=0, M, print1(a(n), ", "));