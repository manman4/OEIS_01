M=8^4-1;

a(n) = fromdigits(Vec(Pol(digits(n, 8))^2)%8, 8); 
for(n=0, M, write("/Users/xxx/Desktop/b170989_01.txt", n, " ", a(n)))