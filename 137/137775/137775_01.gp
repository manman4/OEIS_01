\\ a(n) = (n-1) * (a(n-1) + 3*a(n-2)) with a(0)=1. 
a(n) = if(n<2, 1-n, (n-1)*(a(n-1)+3*a(n-2)));
for(n=0, 20, print1(a(n),", "))