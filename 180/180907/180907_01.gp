\\ G.f.: x/(1-4*x) * (1 - Series_Reversion( x / (1+x+x^2+x^3) )).
my(N=101, x='x+O('x^N)); x/(1-4*x) * (1 - serreverse( x/(1+x+x^2+x^3) ))
my(N=1001, x='x+O('x^N)); Vec(x/(1-4*x) * (1 - serreverse( x/(1+x+x^2+x^3) )))  