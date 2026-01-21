\\ a(n) = Sum_{k=0..n-1} (2*k)!/k! * |Stirling1(n-1+k,2*k)|. 
a(n) = sum(k=0, n-1, (2*k)!/k! * abs(stirling(n-1+k, 2*k, 1)));
for(n=1, 20, print1(a(n), ", "));