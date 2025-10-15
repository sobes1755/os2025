__asm__(".symver sin_3,sin207@@VER_1_3_0");
 
double
sin_3(double x)
{
    double d = x;
    d -= (x*x*x) / (1.*2.*3.);
    d += (x*x*x*x*x) / (1.*2.*3.*4.*5.);
    return d;
}
