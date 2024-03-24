M=500;
a(n) = my(k=1); while(binomial(2*k, k)%n^2>0, k++); k;

for(n=1, M, write("/Users/xxx/Desktop/b111869_1.txt", n, " ", a(n)))