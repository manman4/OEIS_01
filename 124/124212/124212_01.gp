b(n) = n! * polcoef( sqrt( exp(x +x*O(x^n)) / (2 - exp(x +x*O(x^n)))), n);
for(n=0, 17, print1(b(n),", "))   
a(n) = 2^n*b(n);
for(n=0, 17, print1(a(n),", "))   

