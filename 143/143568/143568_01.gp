\\ a(0) = 1; a(n) = (n-1)! * Sum_{k=0..floor((n-1)/4)} (4*k+1) * a(k) * a(n-1-4*k) / (24^k * k! * (n-1-4*k)!). 
a(n) = if(n==0, 1, (n-1)! * sum(k=0, (n-1)\4, (4*k+1) * a(k) * a(n-1-4*k) / (24^k * k! * (n-1-4*k)!)));
for(n=0, 30, print1(a(n),", "))    