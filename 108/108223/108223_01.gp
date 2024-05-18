a(n) = sumdiv(n, d, moebius(n/d) * (n/d)^n * sigma(d, 2*n));
for(n=1, 30, print1(a(n), ", "))
