M=1000;
a(n) = my(cnt=0); for(i=1, n, for(j=1, n\i, for(k=1, n\j, for(l=1, (n-i*j)\k, if(i*j+j*k+k*l<n && (n-(i*j+j*k+k*l))%l==0, cnt++))))); cnt;

for(n=1, M, write("/Users/xxx/Desktop/b191822.txt", n, " ", a(n)))

