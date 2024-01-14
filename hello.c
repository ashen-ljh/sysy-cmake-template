int a=4;

int half(int x,int y, int z) {
  return x / 2;
}
int xx(int x,int y,int z )
{
   return y+z;
}

void f() {}

int main(int a) {
  f();
  int b= xx(1,2,3);
  return half(10,a,5)+b;
}