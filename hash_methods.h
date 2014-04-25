#include <string>
#include <stdint.h>
int bitsize(int);
double geta();
uint64_t hashing (unsigned long long , size_t, double);

//signed ints
unsigned long long hashvalue(char);
unsigned long long hashvalue(int);
unsigned long long hashvalue(long long);
unsigned long long hashvalue(long);

//unsigned ints
unsigned long long hashvalue(unsigned char);
unsigned long long hashvalue(unsigned int);
unsigned long long hashvalue(unsigned long long);
unsigned long long hashvalue(unsigned long);

//decimal values
unsigned long long hashvalue(float);
unsigned long long hashvalue(double);
unsigned long long hashvalue(long double);

//c++ string
unsigned long long hashvalue(std::string const&);
