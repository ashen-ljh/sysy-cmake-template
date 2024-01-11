#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <map>
#include <variant>
#include <stdlib.h>

enum class FuncFParamType { var, list };
enum class UnaryExpType { primary, unary, func_call };
enum class PrimaryExpType { exp, number, lval, list };
enum class StmtType { if_, ifelse, simple, while_ };
enum class SimpleStmtType { lval, exp, block, ret, break_, continue_, list,null };
enum class DeclType { const_decl, var_decl };
enum class ConstInitValType { const_exp, list };
enum class BlockItemType { decl, stmt };
enum class InitValType { exp, list };


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

static std::vector<std::map<std::string, int>> symbol_tables;
static std::vector<std::map<std::string, int>> var_types;
static int level=-1;
static int nowww=0;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual int Calc() const { assert(false); return -1; }
  virtual void dump() const { assert(false); return ;}
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
    std::vector<std::unique_ptr<BaseAST>> block_item_list;
    void Dump() const override {
      level++;
      std::map<std::string, int> symbol_table;
      std::map<std::string, int> var_type;
      symbol_tables.push_back(symbol_table);
      var_types.push_back(var_type);
      if(level==0) std::cout<<"{"<<std::endl;
      if(level==0) std::cout<<"%""entry:"<<std::endl;
      for (auto&& block_item : block_item_list) block_item->Dump();
      if(level==0) std::cout<<"}";
      symbol_tables.pop_back();
      var_types.pop_back();
      level--;

  }
};

class BlockItemAST : public BaseAST
{
public:
    BlockItemType type;
    std::unique_ptr<BaseAST> content;
    void Dump() const override { content->Dump(); }
};

class StmtAST : public BaseAST{
  public:
    SimpleStmtType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> block;
    void Dump() const override {
      if(type==SimpleStmtType::ret)
      {
        exp->Dump();
        std::cout<<" ret %"<<nowww-1<<std::endl;
      }
      else if(type==SimpleStmtType::lval)
      {
        exp->Dump();
        lval->dump();
      }
      else if(type==SimpleStmtType::block)
      {
        block->Dump();
      }
      else if(type==SimpleStmtType::exp)
      {
        exp->Dump();
      }
    }
};

class NumberAST : public BaseAST{
  public:
    int num;
    void Dump() const override {
    std::cout<<" %"<<nowww<<" = add 0, "<<num<<std::endl;
    nowww++;
  }
  int Calc()const override{
      return num;
  }
};

class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> lor_exp;
    void Dump()const override{
      lor_exp->Dump();
    }
    int Calc()const override{
      return lor_exp->Calc();
    }
};

class LOrExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> land_exp;
    int op;
    std::unique_ptr<BaseAST> lor_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==-1)
      {
        land_exp->Dump();
      }
      else
      {
        land_exp->Dump();
        now1=nowww-1;
        lor_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = ne %"<<now1<<", 0"<<std::endl;
        ++nowww;
        std::cout<<" %"<<nowww<<" = ne %"<<now2<<", 0"<<std::endl;
        ++nowww;
        std::cout<<" %"<<nowww<<" = or %"<<nowww-2<<", %"<<nowww-1<<std::endl;
        ++nowww;
      }
    }
    int Calc()const override{
      if(op==-1) return land_exp->Calc();
      else
      {
        int left_v= lor_exp->Calc();
        if(left_v) return 1;
        int right_v=land_exp->Calc();
        if(right_v) return 1;
        return 0;
      }
    }
};

class LAndExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> eq_exp;
    int op;
    std::unique_ptr<BaseAST> land_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==-1)
      {
        eq_exp->Dump();
      }
      else
      {
        eq_exp->Dump();
        now1=nowww-1;
        land_exp->Dump();
        now2=nowww-1;
        std::cout<<" %"<<nowww<<" = ne %"<<now1<<", 0"<<std::endl;
        ++nowww;
        std::cout<<" %"<<nowww<<" = ne %"<<now2<<", 0"<<std::endl;
        ++nowww;
        std::cout<<" %"<<nowww<<" = and %"<<nowww-2<<", %"<<nowww-1<<std::endl;
        ++nowww;
      }
    }
    int Calc()const override{
      if(op==-1) return eq_exp->Calc();
      else{
        int left_v=land_exp->Calc();
        if(left_v==0) return 0;
        int right_v=eq_exp->Calc();
        if(right_v==0) return 0;
        return 1;
      }
    }
};

class EqExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> rel_exp;
    int op;
    std::unique_ptr<BaseAST> eq_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==-1)
      {
        rel_exp->Dump();
      }
      else
      {
        eq_exp->Dump();
        now1=nowww-1;
        rel_exp->Dump();
        now2=nowww-1;
        if(op==Equal)
        {
          std::cout<<" %"<<nowww<<" = eq %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==NotEqual)
        {
          std::cout<<" %"<<nowww<<" = ne %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
      }
    }
    int Calc()const override{
      if(op==-1) return rel_exp->Calc();
      else{
        int left_v=eq_exp->Calc();
        int right_v=rel_exp->Calc();
        if(op==Equal) return left_v==right_v;
        else if(op==NotEqual) return left_v!=right_v;
      }
    }
};

class RelExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> add_exp;
    int op;
    std::unique_ptr<BaseAST> rel_exp;
    void Dump()const override
    {
      int now1,now2;
      if(op==-1)
      {
        add_exp->Dump();
      }
      else
      {
        rel_exp->Dump();
        now1=nowww-1;
        add_exp->Dump();
        now2=nowww-1;
        if(op==Less)
        {
          std::cout<<" %"<<nowww<<" = lt %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==Greater)
        {
          std::cout<<" %"<<nowww<<" = gt %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==LessEq)
        {
          std::cout<<" %"<<nowww<<" = le %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==GreaterEq)
        {
          std::cout<<" %"<<nowww<<" = ge %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
      }
    }
    int Calc()const override{
      if(op==-1) return add_exp->Calc();
      else{
        int left_v=rel_exp->Calc();
        int right_v=add_exp->Calc();
        if(op==Less) return left_v<right_v;
        else if(op==Greater) return left_v>right_v;
        else if(op==LessEq) return left_v<=right_v;
        else if(op==GreaterEq) return left_v>=right_v;
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
      if(op==-1)
      {
        mu_exp->Dump();
      }
      else
      {
        mu_exp->Dump();
        now1=nowww-1;
        add_exp->Dump();
        now2=nowww-1;
        if(op==Add)
        {
          std::cout<<" %"<<nowww<<" = add %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==Sub)
        {
          std::cout<<" %"<<nowww<<" = sub %"<<now2<<", %"<<now1<<std::endl;
          ++nowww;
        }
      }
    }
    int Calc()const override{
      if(op==-1) return mu_exp->Calc();
      else{
        int left_v=add_exp->Calc();
        int right_v=mu_exp->Calc();
        if(op==Add) return left_v+right_v;
        else if(op==Sub) return left_v-right_v;
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
      if(op==-1)
      {
        u_exp->Dump();
      }
      else
      {
        mu_exp->Dump();
        now1=nowww-1;
        u_exp->Dump();
        now2=nowww-1;
        if(op==Mul)
        {
          std::cout<<" %"<<nowww<<" = mul %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==Div)
        {
          std::cout<<" %"<<nowww<<" = div %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
        else if(op==Mod)
        {
          std::cout<<" %"<<nowww<<" = mod %"<<now1<<", %"<<now2<<std::endl;
          ++nowww;
        }
      }
    }
    int Calc()const override{
      if(op==-1) return u_exp->Calc();
      else{
        int left_v=mu_exp->Calc();
        int right_v=u_exp->Calc();
        if(op==Mul) return left_v*right_v;
        else if(op==Div) return left_v/right_v;
        else if(op==Mod) return left_v%right_v;
      }
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
    int Calc()const override{
      if(op==-1||op==NoOperation) return pu_exp->Calc();
      else{
        if(op==Invert) return -pu_exp->Calc();
        else if(op==EqualZero) return !pu_exp->Calc();
      }
    }
};

class PrimaryExpAST : public BaseAST{
  public:
    PrimaryExpType type;
    std::unique_ptr<BaseAST> p_exp;
    void Dump()const override{
      p_exp->Dump();
    }
    int Calc()const override{
      return p_exp->Calc();
    }
};

class DeclAST : public BaseAST{
  public:
    DeclType type;
    std::unique_ptr<BaseAST> decl;
    void Dump()const override{
      decl->Dump();
    }
};

class ConstDeclAST : public BaseAST{
  public:
    std::string b_type;
    std::vector<std::unique_ptr<BaseAST> > const_def_list;
    void Dump() const override
    {
        assert(b_type == "int");
        for (auto&& const_def : const_def_list) const_def->Dump();
    }
};

class ConstDefAST :public BaseAST{
  public:
    std::string ident;
    std::unique_ptr<BaseAST> c_initval;
    int Calc()const override{
      symbol_tables[level][ident]=c_initval->Calc();
      return symbol_tables[level][ident];
    }
    void Dump() const override
    {
      var_types[level][ident]=0;
      Calc();
    }
    
};

class ConstInitValAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> c_exp;
    void Dump() const override
    {
      c_exp->Dump();
    }
    int Calc()const override{
      return c_exp->Calc();
    }
};

class ConstExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    void Dump() const override
    {
      exp->Dump();
    }
    int Calc()const override{
      return exp->Calc();
    }
};

class VarDeclAST : public BaseAST{
  public:
    std::string b_type;
    std::vector<std::unique_ptr<BaseAST> > var_def_list;
    void Dump() const override
    {
        assert(b_type == "int");
        for (auto&& var_def : var_def_list) var_def->Dump();
    }
};

class VarDefAST : public BaseAST{
  public:
    std::string ident;
    bool ifhavev;
    std::unique_ptr<BaseAST> initval;
    void Dump() const override
    {
      var_types[level][ident]=1;
      std::cout<<" @"<<ident<<"_"<<level<<" = alloc i32"<<std::endl;
      if(ifhavev)
      {
        initval->Dump();
        std::cout<<" store %"<<nowww-1<<", @"<<ident<<"_"<<level<<std::endl;
        symbol_tables[level][ident]=initval->Calc();
      }
      else
      {
        symbol_tables[level][ident]=0;
      }
    }
};

class InitValAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    void Dump() const override
    {
      exp->Dump();
    }
    int Calc() const override
    {
      return exp->Calc();
    }
};

class LValAST : public BaseAST{
  public:
    std::string ident;
    void Dump()const override
    {
      for (int i=level;i>=0;--i)
      {
        if(var_types[i].count(ident))
        {
          if(var_types[i][ident]==0)
            std::cout<<" %"<<nowww<<" = add "<<"0 ,"<<symbol_tables[i][ident]<<std::endl;
          else 
            std::cout<<" %"<<nowww<<" = load "<<"@"<<ident<<"_"<<i<<std::endl; 
          nowww++;
          break;
        }
      }
    }
    int Calc() const override
    {
      int cal;
      for (int i=level;i>=0;--i)
      {
        if(symbol_tables[i].count(ident))
          { 
            cal=symbol_tables[i][ident];
            break;
          }
      }
      return cal;
    }
    void dump()const override{
      for (int i=level;i>=0;--i)
      {
        if(var_types[i].count(ident))
          { 
            std::cout<<" store %"<<nowww-1<<", @"<<ident<<"_"<<i<<std::endl;
            break;
          }
      }
    }
};






