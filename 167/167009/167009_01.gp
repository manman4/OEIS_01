M=11;

a(n) = if(n==0, 1, polcoef((1-x)^(n-1)/((1-x)^n-x^n+x*O(x^n^2)), n^2));
for(n=0, M, print1(a(n), ", "));