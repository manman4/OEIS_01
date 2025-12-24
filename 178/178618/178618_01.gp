\\ T(n,k) = Sum_{j=0..k} (-1)^(k-j) * binomial(n+1,k-j) * binomial(n+3*j,3*j).
T(n, k) = sum(j=0, k, (-1)^(k-j)*binomial(n+1, k-j)*binomial(n+3*j, 3*j)); 
for(n=0, 13, for(k=0, n, if(T(n, k) != 0, print1(T(n, k),", "))));