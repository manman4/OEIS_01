FussCatalan_B(steps) = { 
  my(x = 'x, P = 1, x_k = x); 
  for(k = 0, steps - 1, P = P * (1 + x_k); x_k = x * P); 
  return(x * P); 
}

for(s = 0, 5, print(FussCatalan_B(s) + x*O(x^(2^s))));
for(s = 0, 5, print(Vec(FussCatalan_B(s) + x*O(x^(2^s)))));


for(s = 0, 10, print1(polcoef(FussCatalan_B(s) + x*O(x^(2^s)), s + 1),", "));
