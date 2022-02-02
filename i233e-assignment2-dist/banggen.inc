#include <math.h>

// Giga thanks to http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/mt19937ar.c for MT.h
#include "MT.inc"

// Another Giga thanks to http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html

double
Uniform(void)
{
    return genrand_real3();
}

double
rand_Lnormal(double mu, double sigma)
{
   double z= mu + sigma*sqrt(-2.0*log(Uniform()))*sin(2.0*M_PI*Uniform());//gauss random number
   return exp(z);
}

// A little addition by shinoda
int
banggen(double p1, double p2, int psz)
{
	return (int)(rand_Lnormal(p1, p2) * psz);
}

