M=20;

\\ G.f.: g/(1-x^2*g^3) where f/(1-f)^3=x and g=1-f. 
my(N=M, x='x+O('x^N), f=x*sum(k=0, N, (-1)^k*binomial(3*k+3, k)/(k+1)*x^k)); Vec( f/(1-f)^3 - x )
my(N=M, x='x+O('x^N), f=x*sum(k=0, N, (-1)^k*binomial(3*k+3, k)/(k+1)*x^k)); Vec( (1-f)/(1-x^2*(1-f)^3) )