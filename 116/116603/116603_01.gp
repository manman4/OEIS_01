M=10;

\\ G.f.: Product_{k>=1} (1 + k*x)^(1/2^k).
a(n) = my(x='x+O('x^(n+1))); polcoef(prod(k=1, 1000, (1 + k*x)^((1/2)^k)), n) + 0.;
for(n=0, M, print1(round(a(n)),", "));
