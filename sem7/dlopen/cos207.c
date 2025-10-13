#include "tri207.h"

double
cos207(double x)
{
    double d = 1.;
    d -= (x*x) / (1*2);
    d += (x*x*x*x) / (1*2*3*4);
    return d;
}
