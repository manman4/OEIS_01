M=10000;
a(n) = sum(k=0, n, abs(moebius(k^2+1)));
for(n=0, M, write("b124808.txt", n, " ", a(n)));

