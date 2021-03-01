M=20;

a(n) = polcoeff(1/(-1+(n-1)*x-(n-1)*x^2+x*O(x^n)), n-1);
for(n=1, M, print1(a(n), ", "));
