int a=4;

int half(int x,int y, int z) {
  int t=a+x+y+z;
  return t;
}
int xx(int x,int y,int z )
{
   return y+z;
}
int x=2;
void f() {}

int main() {
  f();
  int b= xx(1,2,3);
  return half(10,a,5)+b;
}