__asm__(".symver cos_3,cos207@@VER_1_3_0");

double
cos_3(double x)
{
    double d = 1.;
    d -= (x*x) / (1.*2.);
    d += (x*x*x*x) / (1.*2.*3.*4.);
    return d;
}
