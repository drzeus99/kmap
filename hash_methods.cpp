#include <iostream>
#include <math.h>
#include <stdint.h>
using namespace std;

int bitsize(int byte_size)
{
    return byte_size*8;
}

double geta()
{
    double phi=(sqrt(5)-1)/2;
    return phi;
}

uint64_t hashing (unsigned long long key, size_t m, double a)
{
    return floor(m*(key*a-floor(key*a)));
}

//signed ints
unsigned long long hashvalue(char c)
{return fabs(c);}

unsigned long long hashvalue(int c)
{return fabs(c);}

unsigned long long hashvalue(long c)
{return fabs(c);}

unsigned long long hashvalue(long long c)
{return fabs(c);}

//unsigned ints
unsigned long long hashvalue(unsigned char c) {return c;}
unsigned long long hashvalue(unsigned int c) {return c;}
unsigned long long hashvalue(unsigned long long c) {return c;}
unsigned long long hashvalue(unsigned long c) {return c;}

//floats
unsigned long long hashvalue(float c)
{
    if (c==0)
        return 0;
    else
    {
        int exponent;
        double mantissa=frexp(c,&exponent);
        return (2*fabs(mantissa)-1)*~0ULL;
    }
}

unsigned long long hashvalue(double c)
{
    if (c==0)
        return 0;
    else
    {
        int exponent;
        double mantissa=frexp(c,&exponent);
        return (2*fabs(mantissa)-1)*~0ULL;
    }
}

unsigned long long hashvalue(long double c)
{
    if (c==0)
        return 0;
    else
    {
        int exponent;
        double mantissa=frexp(c,&exponent);
        return (2*fabs(mantissa)-1)*~0ULL;
    }
}

unsigned long long const shift=6;
unsigned long long const mask= ~0ULL <<
    (bitsize(sizeof(unsigned long long))-shift);

unsigned long long hashvalue(std::string const& c)
{
    unsigned long long result=0;
    for(unsigned long long i=0; c[i]!=0; ++i)
        result= (result & mask) ^ (result << shift) ^ c[i];
    return result;
}
