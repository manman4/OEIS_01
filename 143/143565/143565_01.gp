\\ T(0,k) = 1; T(n,k) = (n-1)! * Sum_{j=0..floor((n-1)/k)} (k*j+1) * T(j,k) * T(n-1-k*j,k) / (k!^j * j! * (n-1-k*j)!).
T(n,k) = if(n==0, 1, (n-1)! * sum(j=0, (n-1)\k, (k*j+1) * T(j,k) * T(n-1-k*j,k) / (k!^j * j! * (n-1-k*j)!)));
for(n=1, 10, for(k=1, n, print1(T(n,k), ", ")));