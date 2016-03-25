/* Ratfor code of qnorms, from S:

real function qnorms(pr)
real pr
real p,eta,term,f1,f2,f3,f4,f5,f6
data f1/.010328/
data f2/.802853/
data f3/2.515517/
data f4/.001308/
data f5/.189269/
data f6/1.432788/
if(pr<=0.|pr>=1.)ERROR(`Probability out of range (0,1)')
p = pr
if(p > 0.5) p = 1.0 - pr
#  depending on the size of pr this may error in alog or sqrt
eta=sqrt(-2.*alog(p))
term=((f1*eta+f2)*eta+f3)/(((f4*eta+f5)*eta+f6)*eta+1.0)
if(pr<=.5)return( term - eta )
else return ( eta - term )
end

*/

/*
  cc -o qnorm qnorm.c

 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* adapted from Ratfor code used in S */
double
qnorm(double pr)
{
  double p, eta, term,
    f1 = .010328,
    f2 = .802853,
    f3 = 2.515517,
    f4 = .001308,
    f5 = .189269,
    f6 = 1.432788;

  if(pr <= 0. || pr >= 1.) printf("Probability out of range (0,1): %f", pr);
  p = pr;
    if(p > 0.5) p = 1.0 - pr;
          /*  depending on the size of pr this may error in
              log or sqrt */
  eta  = sqrt(-2.0*log(p));
  term =((f1*eta+f2)*eta+f3)/(((f4*eta+f5)*eta+f6)*eta+1.0);
  if(pr <= .5) return( term - eta );
  else return ( eta - term );
}

main()
{
  int i;
  double p;

  for(i=0; i<100; i++) {
    printf("Enter a probability: ");
    scanf("%lf", &p);
    printf("     normal quantile = %f \n", qnorm(p));
  }
}

