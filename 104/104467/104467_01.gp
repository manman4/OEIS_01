M=100;

a(n) = {x='x+O('x^(n+1)); sum(m=0, sqrt(n), x^(3*m*m)*prod(k=1, 3*m, 1-x^k)/(prod(k=1, m, 1-x^(3*k))*prod(k=1, 2*m, 1-x^(3*k))))};
print(Vec(a(M)));