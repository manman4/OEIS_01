M=9^4-1;

a(n) = fromdigits(Vec(Pol(digits(n, 9))^2)%9, 9);
for(n=0, M, write("/Users/xxx/Desktop/b170990_01.txt", n, " ", a(n)))