#pragma once
#include <iostream>

// 指令级操作
#define NoOperation 0
#define Invert 1
#define EqualZero 2
#define Add 3
#define Sub 4
#define Mul 5
#define Div 6
#define Mod 7
#define Less 8
#define Greater 9
#define LessEq 10
#define GreaterEq 11
#define Equal 12
#define NotEqual 13
#define And 14
#define Or 15
#define NotEqualZero 16

static int nowww=0;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  void Dump() const override {
    std::cout << "fun @";
    func_def->Dump();
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump() const override {
    std::cout << ident<<"(): ";
    func_type->Dump();
    block->Dump();
  }
};

class FuncTypeAST : public BaseAST{
  public:
    std::string type;
    void Dump() const override {
      std::cout<<"i32 ";
  }
};

class BlockAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override {
      std::cout<<"{"<<std::endl;
      std::cout<<"%""entry:"<<std::endl;
      stmt->Dump();
      std::cout<<"}";

  }
};

class StmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    void Dump() const override {
      exp->Dump();
      std::cout<<" ret %"<<nowww-1<<std::endl;
  }
};

class NumberAST : public BaseAST{
  public:
    std::string num;
    void Dump() const override {
    std::cout<<" %"<<nowww<<" = add 0, "<<num<<std::endl;
    nowww++;
  }
};

class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> add_exp;
    void Dump()const override{
      add_exp->Dump();
    }
};

class PrimaryExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> p_exp;
    void Dump()const override{
      p_exp->Dump();
    }
};

class UnaryExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> pu_exp;
    int op;
    void Dump()const override{
      if(op==-1||op==NoOperation) pu_exp->Dump();
      else if(op==Invert){
        pu_exp->Dump();
        std::cout<<" %"<<nowww<<" = sub 0, %"<<nowww-1<<std::endl;
        nowww++;
      } 
      else if(op==EqualZero){
        pu_exp->Dump();
        std::cout<<" %"<<nowww<<" = eq 0, %"<<nowww-1<<std::endl;
        nowww++;
      }
    }
};

class MulExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> mu_exp;
    int op;
    std::unique_ptr<BaseAST> u_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==Mul)
      {
        mu_exp->Dump();
        now1=nowww-1;
        u_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = mul %"<<now1<<", %"<<now2<<std::endl;
        ++nowww;
      }
      else if(op==Div)
      {
        mu_exp->Dump();
        now1=nowww-1;
        u_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = div %"<<now1<<", %"<<now2<<std::endl;
        ++nowww;
      }
      else if(op==Mod)
      {
        mu_exp->Dump();
        now1=nowww-1;
        u_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = mod %"<<now1<<", %"<<now2<<std::endl;
        ++nowww;
      }
      else if(op==-1)
      {
        u_exp->Dump();
      }
    }
};

class AddExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> mu_exp;
    int op;
    std::unique_ptr<BaseAST> add_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==Add)
      {
        mu_exp->Dump();
        now1=nowww-1;
        add_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = add %"<<now1<<", %"<<now2<<std::endl;
        ++nowww;
      }
      else if(op==Sub)
      {
        mu_exp->Dump();
        now1=nowww-1;
        add_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = sub %"<<now2<<", %"<<now1<<std::endl;
        ++nowww;
      }
      else if(op==-1)
      {
        mu_exp->Dump();
      }
    }

};