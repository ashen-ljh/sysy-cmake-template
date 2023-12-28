#pragma once
#include <iostream>
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
    std::unique_ptr<BaseAST> number;
    void Dump() const override {
      std::cout<<" ret ";
      number->Dump();
  }
};

class NumberAST : public BaseAST{
  public:
    std::string num;
    void Dump() const override {
      std::cout<<num<<std::endl;
  }
};

class PrimaryExp:public BaseAST{
  public:
    std::unique_ptr<BaseAST> p_exp;
    void Dump()const override{
      p_exp->Dump();
    }
};
