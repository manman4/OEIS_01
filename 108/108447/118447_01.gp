\\ G.f.: 1 + Series_Reversion( x / ((1+x) * (x+(1+x)^2)) ). 
my(N=40, x='x+O('x^N)); Vec(1 + serreverse( x / ((1+x) * (x+(1+x)^2)) ))  
