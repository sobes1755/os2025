#include "tri207.h"

double
sin207(double x)
{
    double d = x;
    d -= (x*x*x) / (1*2*3);
    d += (x*x*x*x*x) / (1*2*3*4*5);

    return d;
}
