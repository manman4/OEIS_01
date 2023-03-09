M=9999;

a(n) = fromdigits(Vec(Pol(digits(n))^4)%10)
for(n=0, M, write("/Users/xxx/Desktop/b169886_01.txt", n, " ", a(n)))