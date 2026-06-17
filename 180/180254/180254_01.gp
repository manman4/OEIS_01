\\ a(1) = 1; a(n) = Sum_{i,j,k >= 0 and i+j+k=n-2} (n-1)!/((i+1)!*j!*k!) * a(i+1)*a(j+1)*a(k+1).
a(n) = if(n==1, 1, sum(i=0, n-2, sum(j=0, n-2-i, my(k=n-2-i-j); (n-1)!/((i+1)!*j!*k!) * a(i+1)*a(j+1)*a(k+1))));
for(n=1, 11, print1(a(n),", "));

    