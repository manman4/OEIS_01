\\ E.g.f.: (1/x) * Series_Reversion( x*(1 - x)^x ).
my(N=30, x='x+O('x^N)); Vec(serlaplace( serreverse( x*(1 - x)^x )/x ))


 