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
static std::map<std::string, std::string> function_table; 
static std::map<std::string, std::string> function_ret_type;
static std::map<std::string, int> function_param_num;  
static std::vector<int> while_stack;
static int level=0;
static int nowww=0;
static int if_else_num=0;
static int while_num=0;
static std::map<std::string, std::vector<std::string>> function_param_idents;
static std::map<std::string, std::vector<std::string>> function_param_names;
static std::map<std::string, std::vector<std::string>> function_param_types;
static std::string present_func_type;

static int func_num=0;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual int Calc() const { assert(false); return -1; }
  virtual void dump() const { assert(false); return ;}
  virtual std::string Type() const { assert(false); return ""; }
  virtual std::string get_ident() const { assert(false); return ""; }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::vector<std::unique_ptr<BaseAST>> func_def_list;
  std::vector<std::unique_ptr<BaseAST>> decl_list;
  void Dump() const override {
    std::cout << "decl @getint(): i32" << std::endl;
    std::cout << "decl @getch(): i32" << std::endl;
    std::cout << "decl @getarray(*i32): i32" << std::endl;
    std::cout << "decl @putint(i32)" << std::endl;
    std::cout << "decl @putch(i32)" << std::endl;
    std::cout << "decl @putarray(i32, *i32)" << std::endl;
    std::cout << "decl @starttime()" << std::endl;
    std::cout << "decl @stoptime()" << std::endl << std::endl;
    function_table["getint"] = "@getint";
    function_table["getch"] = "@getch";
    function_table["getarray"] = "@getarray";
    function_table["putint"] = "@putint";
    function_table["putch"] = "@putch";
    function_table["putarray"] = "@putarray";
    function_table["starttime"] = "@starttime";
    function_table["stoptime"] = "@stoptime";
    function_ret_type["getint"] = "int";
    function_ret_type["getch"] = "int";
    function_ret_type["getarray"] = "int";
    function_ret_type["putint"] = "void";
    function_ret_type["putch"] = "void";
    function_ret_type["putarray"] = "void";
    function_ret_type["starttime"] = "void";
    function_ret_type["stoptime"] = "void";
    function_param_num["getint"] = 0;
    function_param_num["getch"] = 0;
    function_param_num["getarray"] = 1;
    function_param_num["putint"] = 1;
    function_param_num["putch"] = 1;
    function_param_num["putarray"] = 2;
    function_param_num["starttime"] = 0;
    function_param_num["stoptime"] = 0;
    std::map<std::string, int> global_syms;
    std::map<std::string, int> global_var_type;
    symbol_tables.push_back(global_syms);
    var_types.push_back(global_var_type);
    for (auto&& decl : decl_list) decl->Dump();
      std::cout << std::endl;
    for (auto&& func_def : func_def_list)func_def->Dump();
    symbol_tables.pop_back();
    var_types.pop_back();
  }
};

class FuncFParamAST : public BaseAST{
  public:
    FuncFParamType type;
    std::string b_type;
    std::string ident;
    void Dump() const override{
      assert(b_type == "int");
      std::string param_name="@"+ident;
      std::string name=param_name+"_"+std::to_string(func_num)+"_"+std::to_string(level);
      std::cout<<name;
    }
    std::string get_ident() const override{
      return ident;
    }
    std::string Type() const override{
      return "i32";
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::string func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::vector<std::unique_ptr<BaseAST> > params;
  void Dump() const override {
    func_num++;
    std::string name = "@" + ident;
    function_table[ident] = name;
    function_ret_type[ident] = func_type;
    function_param_num[ident] = params.size();
    present_func_type = function_ret_type[ident];
    std::vector<std::string> idents, names, types;
    std::cout << "fun @"<<ident<<"(";
    for (int i = 0; i < params.size(); i++)
    {
      idents.push_back(params[i]->get_ident());
      params[i]->Dump();
      std::string param_name = "@" + idents.back()+"_"+std::to_string(func_num)+"_"+std::to_string(level);
      names.push_back(param_name);
      types.push_back(params[i]->Type());
      std::cout << ": " << params[i]->Type();
      if (i != params.size() - 1)std::cout << ", ";
    }
    function_param_idents[ident] = move(idents);
    function_param_names[ident] = move(names);
    function_param_types[ident] = move(types);
    std::cout<<")";
    if(func_type=="int") std::cout<<": i32 ";
    block->Dump();
  }
};

class BlockAST : public BaseAST{
  public:
    int c=0;
    std::vector<std::unique_ptr<BaseAST>> block_item_list;
    std::string func="";
    void Dump() const override {
      level++;
      std::map<std::string, int> symbol_table;
      std::map<std::string, int> var_type;
      
      if(level==1) std::cout<<"{"<<std::endl;
      if(level==1) std::cout<<"%""entry:"<<std::endl;
      if (func != "")
      {
        std::vector<std::string> idents = function_param_idents[func];
        std::vector<std::string> names = function_param_names[func];
        std::vector<std::string> types = function_param_types[func];
        for (int i = 0; i < names.size(); i++)
        {
          std::string ident = idents[i];
          std::string name = names[i]; name[0] = '%';
          symbol_table[ident] = func_num;
          var_type[ident] = 2;
          std::cout << " " << name << " = alloc ";
          std::cout << types[i] << std::endl;
          std::cout << " store " << names[i] << ", " << name << std::endl;
        }
      }
      symbol_tables.push_back(symbol_table);
      var_types.push_back(var_type);
      for (auto&& block_item : block_item_list) 
      {
        block_item->Dump();
        if(block_item->Type()=="ret"||block_item->Type()=="break"||block_item->Type()=="cont") 
        {
          c=1;
          break;
        }
      }
      if(c==0) 
      {
        if (func_type == "int")std::cout << " ret 0" << std::endl;
        else if (func_type == "void")std::cout << " ret" << std::endl;
        else assert(false);
      }
      if(level==1) std::cout<<"}"<<std::endl;
      symbol_tables.pop_back();
      var_types.pop_back();
      level--;
  }
  std::string Type() const override{
    for (auto&& block_item : block_item_list)
      if(block_item->Type()=="ret")
        return "ret";
      else if(block_item->Type()=="break")
        return "break";
      else if(block_item->Type()=="cont")
        return "cont";
    return "not";    
  }
};

class BlockItemAST : public BaseAST
{
public:
    BlockItemType type;
    std::unique_ptr<BaseAST> content;
    void Dump() const override { content->Dump(); }
    std::string Type() const override{
      return content->Type();
    }
};

class ComplexStmtAST : public BaseAST{
  public:
    StmtType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> if_stmt;
    std::unique_ptr<BaseAST> else_stmt;
    std::unique_ptr<BaseAST> while_stmt;
    void Dump() const override{
        if(type==StmtType::simple) exp->Dump();
        else if(type==StmtType::if_)
        {
          exp->Dump();
          std::string then_label = "\%then__" + std::to_string(if_else_num);
          std::string end_label = "\%end__" + std::to_string(if_else_num++);
          std::cout << " br %" << nowww-1 << ", " << then_label << ", " << end_label << std::endl;
          std::cout << then_label << ":" << std::endl;
          if_stmt->Dump();
          if(if_stmt->Type()!="ret"&&if_stmt->Type()!="break"&&if_stmt->Type()!="cont") std::cout << " jump " << end_label << std::endl;
          std::cout << end_label << ":" << std::endl;
        }
        else if(type==StmtType::ifelse)
        {
          exp->Dump();
          std::string then_label = "\%then__" + std::to_string(if_else_num);
          std::string else_label = "\%else__" + std::to_string(if_else_num);
          std::string end_label = "\%end__" + std::to_string(if_else_num++);
          std::cout << " br %" << nowww-1 << ", " << then_label << ", " << else_label << std::endl;
          std::cout << then_label << ":" << std::endl;
          if_stmt->Dump();
          if(if_stmt->Type()!="ret"&&if_stmt->Type()!="break"&&if_stmt->Type()!="cont") std::cout << " jump " << end_label << std::endl;
          std::cout << else_label << ":" << std::endl;
          else_stmt->Dump();
          if(else_stmt->Type()!="ret"&&else_stmt->Type()!="break"&&else_stmt->Type()!="cont") std::cout << " jump " << end_label << std::endl;
          if(!((if_stmt->Type()=="ret"||if_stmt->Type()=="break"||if_stmt->Type()=="cont")&&(else_stmt->Type()=="ret"||else_stmt->Type()=="break"||else_stmt->Type()=="cont")))
            std::cout << end_label << ":" << std::endl;
        }
        else if(type==StmtType::while_)
        {
          std::string entry_label = "\%while__" + std::to_string(while_num);
          std::string body_label = "\%do__" + std::to_string(while_num);
          std::string end_label = "\%while_end__" + std::to_string(while_num);
          while_stack.push_back(while_num++);
          std::cout << " jump " << entry_label << std::endl;
          std::cout << entry_label << ":" << std::endl;
          exp->Dump();
          std::cout << " br %" << nowww-1 << ", " << body_label << ", " << end_label << std::endl;
          std::cout << body_label << ":" << std::endl;
          while_stmt->Dump();
          if (while_stmt->Type() != "ret" && while_stmt->Type() != "break" && while_stmt->Type() != "cont")
            std::cout << " jump " << entry_label << std::endl;
          std::cout << end_label << ":" << std::endl;
          while_stack.pop_back();
        }
    }
    std::string Type() const override{
      if(type==StmtType::simple) return exp->Type();
      else if(type==StmtType::if_) return "not";
      else if(type==StmtType::ifelse){
        if((if_stmt->Type()=="ret"||if_stmt->Type()=="break"||if_stmt->Type()=="cont")&&(else_stmt->Type()=="ret"||else_stmt->Type()=="break"||else_stmt->Type()=="cont"))
          return "ret";
        else return "not";
      }
      else if(type==StmtType::while_) return "not";
      
    }
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
      else if(type==SimpleStmtType::break_)
      {
        assert(!while_stack.empty());
        int while_no = while_stack.back();
        std::string end_label = "\%while_end__" + std::to_string(while_no);
        std::cout << " jump " << end_label << std::endl;
      }
      else if(type==SimpleStmtType::continue_)
      {
        assert(!while_stack.empty());
        int while_no = while_stack.back();
        std::string entry_label = "\%while__" + std::to_string(while_no);
        std::cout << " jump " << entry_label << std::endl;
      }
    }
    std::string Type() const override{
      if(type==SimpleStmtType::ret) return "ret";
      else if(type==SimpleStmtType::block) return block->Type();
      else if(type==SimpleStmtType::break_) return "break";
      else if(type==SimpleStmtType::continue_) return "cont";
      else return "not";
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
    UnaryExpType type;
    std::unique_ptr<BaseAST> pu_exp;
    std::string ident;
    std::vector<std::unique_ptr<BaseAST> > params;
    int op;
    void Dump()const override{
      if(type!=UnaryExpType::func_call)
      {
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
      else
      {
        std::vector<int> param_vars;
        for (auto&& param : params)
        {
          param->Dump();
          param_vars.push_back(nowww-1);
        }
        assert(function_table.count(ident));
        assert(function_param_num[ident] == params.size());
        if(function_ret_type[ident]=="int") std::cout<<" %"<<nowww<<" = ";
        std::cout<<" call "<<function_table[ident]<<"(";
        for(int i=0;i<param_vars.size();++i)
        {
          std::cout<<"%"<<param_vars[i];
          if(i!=param_vars.size()-1) std::cout<<",";
        }
        std::cout<<")"<<std::endl;
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
    std::string Type() const override{
      return "notret";
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
      symbol_tables[level][ident]=func_num;
      if(ifhavev)
      {
        std::cout<<" @"<<ident<<"_"<<func_num<<"_"<<level<<" = alloc i32"<<std::endl;
        initval->Dump();
        std::cout<<" store %"<<nowww-1<<", @"<<ident<<"_"<<func_num<<"_"<<level<<std::endl;
      }
      else
      {
        if(level==0)
          std::cout<<" @"<<ident<<"_"<<func_num<<"_"<<level<<" = alloc i32, zeroinit"<<std::endl;
        else std::cout<<" @"<<ident<<"_"<<func_num<<"_"<<level<<" = alloc i32"<<std::endl;
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
          else if(var_types[i][ident]==1)
            std::cout<<" %"<<nowww<<" = load "<<"@"<<ident<<"_"<<symbol_tables[i][ident]<<"_"<<i<<std::endl;
          else
            std::cout<<" %"<<nowww<<" = load "<<"%"<<ident<<"_"<<symbol_tables[i][ident]<<"_"<<i<<std::endl;
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
            if(var_types[i][ident]!=2)
              std::cout<<" store %"<<nowww-1<<", @"<<ident<<"_"<<func_num<<"_"<<i<<std::endl;
            else
              std::cout<<" store %"<<nowww-1<<", %"<<ident<<"_"<<func_num<<"_"<<i<<std::endl;
            break;
          }
      }
    }
};






