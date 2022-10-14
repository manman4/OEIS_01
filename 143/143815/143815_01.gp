M=15;

Bell_poly(n, x) = exp(-x)*suminf(k=0, k^n*x^k/k!);
a(n) = my(w=(-1+sqrt(3)*I)/2); round(Bell_poly(n, 1)+Bell_poly(n, w)+Bell_poly(n, w^2))/3;
for(n=0, M, print1(a(n), ", "));