M=5^5-1;

a(n) = fromdigits(Vec(Pol(digits(n, 5))^2)%5, 5); 
for(n=0, M, write("/Users/xxx/Desktop/b170986_01.txt", n, " ", a(n)))