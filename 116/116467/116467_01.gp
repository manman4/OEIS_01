M=100;

a(n) = sum(k=0, n, polcoeff(polcoeff(1/(1-x*y)+ x*(1+x-2*x^2*y)/(1-x)/(1+x+x*y+x*O(x^n)+y*O(y^k))/(1-x*y), n, x), k, y)^2);
for(n=0, M, print1(a(n),", "));

\\ G.f.: (1/(1-x)) * (1 + x + 4*x^2*(1-sqrt(1-4*x))/(3-sqrt(1-4*x))^2).
b(n) = my(N=M, x='x+O('x^(N+10))); polcoef((1/(1-x)) * (1 + x + 4*x^2*(1-sqrt(1-4*x))/(3-sqrt(1-4*x))^2), n);
for(n=0, M, print1(a(n)-b(n),", "));     