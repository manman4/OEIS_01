a343773(n) = sum(k=0, n\2, (-1)^k*binomial(n, 2*k)*binomial(2*k, k)/(k+1));
for(n=0, 28, print1(a343773(n), ", "))

\\ a(n+1) = binomial(n+2,2) * A343773(n)
a(n) = if(n==0, 0, binomial(n+1,2)*a343773(n-1));
for(n=0, 28, print1(a(n), ", ")) 