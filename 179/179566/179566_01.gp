M=450;

BIN=matrix(M+1,M+1,i,j,0);
for(n=0,M,BIN[n+1,1]=1;BIN[n+1,n+1]=1;for(k=1,n-1,BIN[n+1,k+1]=BIN[n,k]+BIN[n,k+1]));

EU=matrix(M+1,M+1,i,j,0);
EU[1,1]=1;
for(n=1,M,for(k=0,n-1,EU[n+1,k+1]=(k+1)*EU[n,k+1]+if(k>0,(n-k)*EU[n,k],0)));

D=matrix(M+1,M+1,i,j,0);
for(n=0,M,for(k=0,n,my(s=0,j0=if(k>0,k,0));for(j=j0,n,s+=(-1)^(n-j)*BIN[n+1,j+1]*EU[j+1,k+1]);D[n+1,k+1]=s));

EVEN=vector(M+1,i,0);
for(m=0,M,if(m%2==0,EVEN[m+1]=D[m+1,m/2+1]));

E(n,k)=if(n<0||k<0||k>n,0,EU[n+1,k+1]);
a271697(n,k)=if(n<0||k<0||k>n,0,D[n+1,k+1]);
a(n)={my(s=0);for(m=0,n,if(m%2==0,s+=BIN[n+1,m+1]*EVEN[m+1]));s;};

write("b179566_1.txt",0," ",1);
cnt=1;
for(n=1,M,write("b179566_1.txt",cnt," ",(n!+a(n))/2);cnt++);
