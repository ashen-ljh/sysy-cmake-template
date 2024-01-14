%code requires {
  #include <memory>
  #include <string>
  #include "AST.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "AST.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST> > *vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN LOR LAND EQ NEQ GEQ LEQ LQ GQ CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef Block Stmt Number PrimaryExp Exp UnaryExp MulExp AddExp LOrExp RelExp EqExp LAndExp
%type <ast_val> BlockItem Decl LVal ConstDecl ConstDef ConstInitVal ConstExp VarDecl VarDef InitVal ComplexStmt
%type <ast_val> OpenStmt ClosedStmt  FuncFParam CompUnitList
%type <int_val> UnaryOp
%type <vec_val> BlockItemList ConstDefList VarDefList FuncFParams FuncRParms  
%type <str_val> Type 
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : CompUnitList{
      auto comp_unit=unique_ptr<BaseAST>($1);
      ast=move(comp_unit);
  } 
  ;

CompUnitList
  : FuncDef{
      auto ast=new CompUnitAST();
      auto func_def=unique_ptr<BaseAST>($1);
      ast->func_def_list.push_back(move(func_def));
      $$=ast;
  }|Decl{
      auto ast=new CompUnitAST();
      auto decl=unique_ptr<BaseAST>($1);
      ast->decl_list.push_back(move(decl));
      $$=ast;
  }|CompUnitList FuncDef{
      auto ast=(CompUnitAST*)($1);
      auto func_def=unique_ptr<BaseAST>($2);
      ast->func_def_list.push_back(move(func_def));
      $$=ast;
  }|CompUnitList Decl{
      auto ast=(CompUnitAST*)($1);
      auto decl=unique_ptr<BaseAST>($2);
      ast->decl_list.push_back(move(decl));
      $$=ast;
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : Type IDENT '(' ')' Block {
    auto ast=new FuncDefAST();
    ast->func_type = *unique_ptr<string>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    cout<<"FuncDef"<<endl;
    $$ = ast;
  }|Type IDENT '(' FuncFParams ')' Block{
    auto ast=new FuncDefAST();
    ast->func_type = *unique_ptr<string>($1);
    ast->ident = *unique_ptr<string>($2);
    vector<unique_ptr<BaseAST>> *v_ptr = ($4);
    for (auto it = v_ptr->begin(); it != v_ptr->end(); it++)
      ast->params.push_back(move(*it));
    ast->block = unique_ptr<BaseAST>($6);
    cout<<"FuncDef"<<endl;
    ((BlockAST*)(ast->block).get())->func = ast->ident;
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam{
      vector<unique_ptr<BaseAST>> *v = new vector<unique_ptr<BaseAST>>;
      v->push_back(unique_ptr<BaseAST>($1));
      $$ = v;
  }| FuncFParams ',' FuncFParam{
      vector<unique_ptr<BaseAST>> *v = ($1);
      v->push_back(unique_ptr<BaseAST>($3));
      $$ = v;
  }
  ;

FuncFParam
  : Type IDENT{
      auto ast = new FuncFParamAST();
      ast->type = FuncFParamType::var;
      ast->b_type = *unique_ptr<string>($1);
      ast->ident = *unique_ptr<string>($2);
      $$ = ast;
  }
  ;

FuncRParms
  : Exp{
      vector<unique_ptr<BaseAST>> *v = new vector<unique_ptr<BaseAST>>;
      v->push_back(unique_ptr<BaseAST>($1));
      $$ = v;
  }|FuncRParms ',' Exp{
      vector<unique_ptr<BaseAST>> *v = ($1);
      v->push_back(unique_ptr<BaseAST>($3));
      $$ = v;
  }
  ;

// 同上, 不再解释

Block
  : '{' BlockItemList '}' {
    auto ast=new BlockAST();
    vector<unique_ptr<BaseAST>> *v_ptr = ($2);
    for(auto it=v_ptr->begin();it!=v_ptr->end();it++)
      ast->block_item_list.push_back(move(*it));
    cout<<"Block"<<endl;
    $$ = ast;
  }
  ;

ComplexStmt
  : OpenStmt{
    auto ast= ($1);
    cout<<"ComplexStmt:Open"<<endl;
    $$=ast;
  }|ClosedStmt{
    auto ast= ($1);
    cout<<"ComplexStmt:Closed"<<endl;
    $$=ast;
  }
  ;

ClosedStmt
  : Stmt{
      auto ast=new ComplexStmtAST();
      ast->type=StmtType::simple;
      ast->exp=unique_ptr<BaseAST>($1);
      cout<<"ClosedStmt:stmt"<<endl;
      $$=ast;
  }|IF '(' Exp ')' ClosedStmt ELSE ClosedStmt{
      auto ast=new ComplexStmtAST();
      ast->type=StmtType::ifelse;
      ast->exp=unique_ptr<BaseAST>($3);
      ast->if_stmt=unique_ptr<BaseAST>($5);
      ast->else_stmt=unique_ptr<BaseAST>($7);
      cout<<"ClosedStmt:ifelse"<<endl;
      $$=ast;
  }|WHILE '(' Exp ')' ClosedStmt{
      auto ast=new ComplexStmtAST();
      ast->type=StmtType::while_;
      ast->exp=unique_ptr<BaseAST>($3);
      ast->while_stmt=unique_ptr<BaseAST>($5);
      $$=ast;
  }
  ;
  
OpenStmt
  : IF '(' Exp ')' ComplexStmt{
      auto ast=new ComplexStmtAST();
      ast->type = StmtType::if_;
      ast->exp=unique_ptr<BaseAST>($3);
      ast->if_stmt= unique_ptr<BaseAST>($5);
      cout<<"OpenStmt:if_"<<endl;
      $$=ast;
  }|IF '(' Exp ')' ClosedStmt ELSE OpenStmt{
      auto ast=new ComplexStmtAST();
      ast->type=StmtType::ifelse;
      ast->exp=unique_ptr<BaseAST>($3);
      ast->if_stmt=unique_ptr<BaseAST>($5);
      ast->else_stmt=unique_ptr<BaseAST>($7);
      cout<<"OpenStmt:ifelse"<<endl;
      $$=ast;
  }|WHILE '(' Exp ')' OpenStmt{
      auto ast=new ComplexStmtAST();
      ast->type=StmtType::while_;
      ast->exp=unique_ptr<BaseAST>($3);
      ast->while_stmt=unique_ptr<BaseAST>($5);
      $$=ast;
  }
  ;



Stmt
  : RETURN Exp ';' {
    auto ast=new StmtAST();
    ast->type = SimpleStmtType::ret;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|RETURN ';'{

  }|LVal '=' Exp ';'{
    auto ast = new StmtAST();
    ast->type = SimpleStmtType::lval;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|Block{
    auto ast=new StmtAST();
    ast->type=SimpleStmtType::block;
    ast->block=unique_ptr<BaseAST>($1);
    $$=ast;
  }|Exp';'{
    auto ast=new StmtAST();
    ast->type=SimpleStmtType::exp;
    ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|';'{
    auto ast=new StmtAST();
    ast->type=SimpleStmtType::null;
    $$=ast;
  }|BREAK ';'{
      auto ast=new StmtAST();
      ast->type=SimpleStmtType::break_;
      $$=ast;
  }|CONTINUE ';'{
      auto ast=new StmtAST();
      ast->type=SimpleStmtType::continue_;
      $$=ast;
  }
  ;

Exp
  : LOrExp {
    auto ast= new ExpAST();
    ast->lor_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

LOrExp
  : LAndExp{
      auto ast=new LOrExpAST();
      ast->land_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|LOrExp LOR LAndExp{
      auto ast=new LOrExpAST();
      ast->lor_exp=unique_ptr<BaseAST>($1);
      ast->land_exp=unique_ptr<BaseAST>($3);
      ast->op=Or;
      $$=ast;
  }
  ;

  LAndExp
  : EqExp{
      auto ast=new LAndExpAST();
      ast->eq_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|LAndExp LAND EqExp{
      auto ast=new LAndExpAST();
      ast->land_exp=unique_ptr<BaseAST>($1);
      ast->eq_exp=unique_ptr<BaseAST>($3);
      ast->op=And;
      $$=ast;
  }
  ;

  EqExp
  : RelExp{
      auto ast=new EqExpAST();
      ast->rel_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|EqExp EQ RelExp{
      auto ast=new EqExpAST();
      ast->eq_exp=unique_ptr<BaseAST>($1);
      ast->rel_exp=unique_ptr<BaseAST>($3);
      ast->op=Equal;
      $$=ast;
  }|EqExp NEQ RelExp{
      auto ast=new EqExpAST();
      ast->eq_exp=unique_ptr<BaseAST>($1);
      ast->rel_exp=unique_ptr<BaseAST>($3);
      ast->op=NotEqual;
      $$=ast;
  }
  ;

  RelExp
  : AddExp{
      auto ast=new RelExpAST();
      ast->add_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|RelExp LQ AddExp{
      auto ast=new RelExpAST();
      ast->rel_exp=unique_ptr<BaseAST>($1);
      ast->add_exp=unique_ptr<BaseAST>($3);
      ast->op=Less;
      $$=ast;
  }|RelExp GQ AddExp{
      auto ast=new RelExpAST();
      ast->rel_exp=unique_ptr<BaseAST>($1);
      ast->add_exp=unique_ptr<BaseAST>($3);
      ast->op=Greater;
      $$=ast;
  }|RelExp LEQ AddExp{
      auto ast=new RelExpAST();
      ast->rel_exp=unique_ptr<BaseAST>($1);
      ast->add_exp=unique_ptr<BaseAST>($3);
      ast->op=LessEq;
      $$=ast;
  }|RelExp GEQ AddExp{
      auto ast=new RelExpAST();
      ast->rel_exp=unique_ptr<BaseAST>($1);
      ast->add_exp=unique_ptr<BaseAST>($3);
      ast->op=GreaterEq;
      $$=ast;
  }
  ;

  AddExp
  : MulExp{
      auto ast=new AddExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|AddExp '+' MulExp{
      auto ast=new AddExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($3);
      ast->add_exp=unique_ptr<BaseAST>($1);
      ast->op=Add;
      $$=ast;
  }|AddExp '-' MulExp{
      auto ast=new AddExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($3);
      ast->add_exp=unique_ptr<BaseAST>($1);
      ast->op=Sub;
      $$=ast;
  }
  ;

  MulExp
  : UnaryExp {
      auto ast=new MulExpAST();
      ast->u_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|MulExp '*' UnaryExp{
      auto ast=new MulExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($1);
      ast->u_exp=unique_ptr<BaseAST>($3);
      ast->op=Mul;
      $$=ast;
  }|MulExp '/' UnaryExp{
      auto ast=new MulExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($1);
      ast->u_exp=unique_ptr<BaseAST>($3);
      ast->op=Div;
      $$=ast;
  }|MulExp '%' UnaryExp{
      auto ast=new MulExpAST();
      ast->mu_exp=unique_ptr<BaseAST>($1);
      ast->u_exp=unique_ptr<BaseAST>($3);
      ast->op=Mod;
      $$=ast;
  }
  ;

  UnaryExp
  : PrimaryExp{
      auto ast=new UnaryExpAST();
      ast->type=UnaryExpType::primary;
      ast->pu_exp=unique_ptr<BaseAST>($1);
      ast->op=-1;
      $$=ast;
  }|UnaryOp UnaryExp{
      auto ast=new UnaryExpAST();
      ast->type=UnaryExpType::unary;
      ast->pu_exp=unique_ptr<BaseAST>($2);
      ast->op=$1;
      $$=ast;
  }|IDENT '(' FuncRParms ')' {
      auto ast=new UnaryExpAST();
      ast->type=UnaryExpType::func_call;
      vector<unique_ptr<BaseAST>> *v_ptr = ($3);
      ast->ident=*unique_ptr<string>($1);
      for (auto it = v_ptr->begin(); it != v_ptr->end(); it++)
        ast->params.push_back(move(*it));
      $$=ast;
  }|IDENT '(' ')'{
      auto ast=new UnaryExpAST();
      ast->type=UnaryExpType::func_call;
      ast->ident=*unique_ptr<string>($1);
      $$=ast;
  }
  ;

PrimaryExp
  :'(' Exp ')'{
    auto ast=new PrimaryExpAST();
    ast->type = PrimaryExpType::exp;
    ast->p_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }|Number {
      auto ast=new PrimaryExpAST();
      ast->type = PrimaryExpType::number;
      ast->p_exp=unique_ptr<BaseAST>($1);
      $$=ast;
  }|LVal{
      auto ast = new PrimaryExpAST();
      ast->type = PrimaryExpType::lval;
      ast->p_exp = unique_ptr<BaseAST>($1);
      $$ = ast;
  }
  ;  


UnaryOp
  : '+'{
    $$ = NoOperation;
  }|'-'{
    $$ = Invert;
  }|'!'{
    $$ = EqualZero;
  }
  ;

Decl
  : ConstDecl{
    auto ast = new DeclAST();
    ast->type = DeclType::const_decl;
    ast->decl = unique_ptr<BaseAST>($1);
    cout<<"Decl:Const"<<endl;
    $$ = ast;
  }|VarDecl{
    auto ast = new DeclAST();
    ast->type = DeclType::var_decl;
    ast->decl = unique_ptr<BaseAST>($1);
    cout<<"Decl:Var"<<endl;
    $$ = ast;
  }
  ;

ConstDecl
  : CONST Type ConstDefList ';'{
    auto ast=new ConstDeclAST();
    ast->b_type=*unique_ptr<string>($2);
    vector<unique_ptr<BaseAST> > *v_ptr = ($3);
    for (auto it = v_ptr->begin(); it != v_ptr->end(); it++)
      ast->const_def_list.push_back(move(*it));
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal{
    auto ast=new ConstDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->c_initval=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

ConstInitVal
  : ConstExp{
    auto ast=new ConstInitValAST();
    ast->c_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

BlockItem
  : Decl {
      auto ast = new BlockItemAST();
      ast->type = BlockItemType::decl;
      ast->content = unique_ptr<BaseAST>($1);
      $$ = ast;
  }|ComplexStmt {
     auto ast = new BlockItemAST();
     ast->type = BlockItemType::stmt;
     ast->content = unique_ptr<BaseAST>($1);
     $$ = ast;
  }
  ;

ConstExp
  : Exp{
    auto ast=new ConstExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

VarDecl
  : Type VarDefList ';'{
    auto ast=new VarDeclAST();
    ast->b_type=*unique_ptr<string>($1);
    vector<unique_ptr<BaseAST> > *v_ptr = ($2);
    for (auto it = v_ptr->begin(); it != v_ptr->end(); it++)
      ast->var_def_list.push_back(move(*it));
    $$ = ast;
  }
  ;

VarDef
  : IDENT{
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->ifhavev = false;
    $$ = ast;
  }|IDENT '=' InitVal{
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->ifhavev = true;
    ast->initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

InitVal
  : Exp{
    auto ast = new InitValAST();
    ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

BlockItemList
  : {
      vector<unique_ptr<BaseAST>> *v = new vector<unique_ptr<BaseAST>>;
      $$ = v;
    }|BlockItemList BlockItem {
        vector<unique_ptr<BaseAST>> *v = ($1);
        v->push_back(unique_ptr<BaseAST>($2));
        $$ = v;
    }
    ;

ConstDefList
  : ConstDef {
    vector<unique_ptr<BaseAST>> *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }|ConstDefList ',' ConstDef {
      vector<unique_ptr<BaseAST>> *v = ($1);
      v->push_back(unique_ptr<BaseAST>($3));
      $$ = v;
  }
  ;

VarDefList
  : VarDef {
    vector<unique_ptr<BaseAST>> *v = new vector<unique_ptr<BaseAST> >;
    v->push_back(unique_ptr<BaseAST>($1));
    $$ = v;
  }|VarDefList ',' VarDef{
    vector<unique_ptr<BaseAST>> *v = ($1);
    v->push_back(unique_ptr<BaseAST>($3));
    $$ = v;
  }
  ;

Number
  : INT_CONST {
    auto ast=new NumberAST();
    ast->num=$1;
    $$ = ast;
  }
  ;

LVal
  : IDENT{
    auto ast = new LValAST();
    ast->ident=*unique_ptr<string>($1);
    $$=ast;
  }
  ;

Type
  : INT{
    string *type = new string("int");
    $$ = type;
  }|VOID{
    string *type=new string("void");
    $$=type;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
