#include "math.h"
#include "errno.h"

extern int errno;

#define P0 -0.35181283430177117881e+6
#define P1 -0.11563521196851768270e+5
#define P2 -0.16375798202630751372e+3
#define P3 -0.78966127417357099479e+0
#define Q0 -0.21108770058106271242e+7
#define Q1 +0.36162723109421836460e+5
#define Q2 -0.27773523119650701667e+3

#define PS(x) (((P3*x P2)*x P1)*x P0)
#define QS(x) (((x Q2)*x Q1)*x Q0)

double sinh(x)
double x;
{
	double y, w, z;
	int sign;
	
	y = x;
	sign = 0;
	if (x < 0.0) {
		y = -x;
		sign = 1;
	}
	if (y > 1.0) {
		w = y - 0.6931610107421875000;
		if (w > 349.3) {
			errno = ERANGE;
			z = HUGE;
		} else {
			z = exp(w);
			if (w < 19.95)
				z -= 0.24999308500451499336 / z;
			z += 0.13830277879601902638e-4 * z;
		}
		if (sign)
			z = -z;
	} else if (y < 2.3e-10)
		z = x;
	else {
		z = x*x;
		z = x + x *
				(z*(PS(z)
				/QS(z)));
	}
	return z;
}

double cosh(x)
double x;
{
	double y, w, z;
	
	y = fabs(x);
	if (y > 1.0) {
		w = y - 0.6931610107421875000;
		if (w > 349.3) {
			errno = ERANGE;
			return HUGE;
		}
		z = exp(w);
		if (w < 19.95)
			z += 0.24999308500451499336 / z;
		z += 0.13830277879601902638e-4 * z;
	} else {
		z = exp(y);
		z = z*0.5 + 0.5/z;
	}
	return z;
}
