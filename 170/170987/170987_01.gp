M=6^5-1;

a(n) = fromdigits(Vec(Pol(digits(n, 6))^2)%6, 6); 
for(n=0, M, write("/Users/xxx/Desktop/b170987_01.txt", n, " ", a(n)))