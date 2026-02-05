\\ a(n) = Sum_{k=0..n-1} a(k) * a(n-1-k) if n is odd, otherwise a(n) = a(n/2) + Sum_{k=0..n-1} a(k) * a(n-1-k). 
a(n) = if(n==0, 1, if(n%2==0, a(n/2) + sum(k=0, n-1, a(k)*a(n-1-k)), sum(k=0, n-1, a(k)*a(n-1-k)) ));
for(n=0, 20, print1(a(n),", "));  



