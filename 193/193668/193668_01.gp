\\ Recurrence: a(n) = (n+2)*a(n-1) - (n-2)*a(n-2) for n > 2.
a(n) = my(v=[1,1,5]); if(n<3, v[n+1], (n+2)*a(n-1)-(n-2)*a(n-2));
for(n=0, 15, print1(a(n),", "));

\\ 以下はダメ
a(n) = my(v=[1,1]); if(n<2, v[n+1], (n+2)*a(n-1)-(n-2)*a(n-2));
for(n=0, 15, print1(a(n),", "));