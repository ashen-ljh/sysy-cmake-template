int a=1;
const int b=8;
int pow(int x) {
  if(x==0) return 1;
  return 2*pow(x-1);
}
int xx(int x,int y,int z )
{
   return y+z;
}
int x;
void f() {}

int main() {
  int w=15;
  f();
  
  return pow(a)+pow(b);
}