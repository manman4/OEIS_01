M=4095;

a(n) = fromdigits(Vec(Pol(digits(n, 4))^2)%4, 4); 
for(n=0, M, write("/Users/xxx/Desktop/b170985_01.txt", n, " ", a(n)))