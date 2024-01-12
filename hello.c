int main() {
  int a = 2;
  if (a) {
    return a+2;
  } else return a+1;  // 在实际写 C/C++ 程序的时候别这样, 建议 if 的分支全部带大括号
  return a;
}