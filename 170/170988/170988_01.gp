M=7^4-1;

a(n) = fromdigits(Vec(Pol(digits(n, 7))^2)%7, 7); 
for(n=0, M, write("/Users/xxx/Desktop/b170988_01.txt", n, " ", a(n)))