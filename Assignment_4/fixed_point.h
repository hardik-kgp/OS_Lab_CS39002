// taking lower 14 bits as fractional bits  

#define f (1 << 14)
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))

int convertN (int n){ return n * f;}
int convertX_zero (int x){ return x / f;}
int convertX_int (int x){
  if (x >= 0)
    return (x + f / 2) / f;
  else	
    return (x - f / 2) / f;
}
int add_xy(int x, int y){ return x + y;}
int sub_xy (int x, int y){ return x - y;}
int add_xn (int x, int n){ return x + n * f;	}
int sub_xn (int x, int n){ return x - n * f; }
int mult_xy (int x, int y){ return ((int64_t)x) * y / f;}
int mult_xn (int x, int y){ return x * y;}
int div_xy (int x, int y){ return ((int64_t)x) * f / y;	}
int div_xn (int x, int y){ return x / y;}