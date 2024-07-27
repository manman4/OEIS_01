M=1000;
D(x, y, n) = sum(k=1, n-1, sigma(k, x)*sigma(n-k, y));
a(n) = sigma(n, 2) - n*sigma(n, 0) - D(0, 0, n);

for(n=1, M, write("/Users/xxx/Desktop/b191822_1.txt", n, " ", a(n)))

