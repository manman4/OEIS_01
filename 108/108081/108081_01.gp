a(n) = polcoef(1/((1-x-x^2) * (1-x)^n + x*O(x^n)), n); 
for(n=0, 20, print1(a(n),", "))