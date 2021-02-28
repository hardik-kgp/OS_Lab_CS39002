/*x and y are fixed-point numbers, n is an integer,
 fixed-point numbers are in signed p.q format
 where p + q = 31, and f is 1 << q:*/
#include <stdint.h>
#define f (1<<14) // q = 14

int
convertN(int n)
{
  return n * f;
}

int
convertX_zero(int x)
{
  return x / f;
}

int
convertX_near(int x)
{
  return x>=0 ? (x+f/2)/f : (x-f/2)/f;
}

int
add_xy(int x, int y)
{
  return x+y;
}

int
sub_xy(int x, int y)
{
  return x-y;
}

int
add_xn(int x, int n)
{
  return x+n*f;
}

int
sub_xn(int x, int n)
{
  return x-n*f;
}

int
mult_xy(int x, int y)
{
  return ((int64_t)x)*y/f;
}

int
div_xy(int x, int y)
{
  return ((int64_t)x)*f/y;
}

int
mult_xn(int x, int n)
{
  return x*n;
}

int
div_xn(int x, int n)
{
  return x/n;
}
