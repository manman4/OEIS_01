\\ a(0) = 1; a(n) = Sum_{x_1, x_2, ..., x_9>=0 and x_1+x_2+...+x_9=n-1} (-1)^(x_1+x_2+x_3+x_4) * Product_{k=1..9} a(x_k). 
a(n) = if(n==0, 1, sum(x1=0, n-1, sum(x2=0, n-1-x1, sum(x3=0, n-1-x1-x2, sum(x4=0, n-1-x1-x2-x3, sum(x5=0, n-1-x1-x2-x3-x4, sum(x6=0, n-1-x1-x2-x3-x4-x5, sum(x7=0, n-1-x1-x2-x3-x4-x5-x6, sum(x8=0, n-1-x1-x2-x3-x4-x5-x6-x7, (-1)^(x1+x2+x3+x4) * a(x1) * a(x2) * a(x3) * a(x4) * a(x5) * a(x6) * a(x7) * a(x8) * a(n-1-x1-x2-x3-x4-x5-x6-x7-x8) )))))))) );
for(n=0, 6, print1(a(n),", "));
