#include <stdio.h>
#include <iostream>
#include "rdtsc.h"
#include <math.h>

int main(int argc, char* argv[])
{
 
 unsigned long long a,b;
 int c=0;
 
 a = rdtsc();
 for(int i=0;i<100;i++) c++;
 b = rdtsc();
 
 // printf("%llu\n", b-a);
 std::cout << a ;
 //std::cout << b << endl;
 //std::cout << b-a << endl;
 return 0;
}
