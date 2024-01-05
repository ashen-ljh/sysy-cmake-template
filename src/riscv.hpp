#pragma once
#include <iostream>
#include <string>
#include <cassert>
#include <map>
#include <cmath>
#include "koopa.h"
struct Reg { int reg_name; int reg_offset; };//寄存器结构体
std::string reg_names[16] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "x0"};//寄存器名称
koopa_raw_value_t registers[16];//寄存器对应的指令
int reg_stats[16] = {0};//寄存器状态
koopa_raw_value_t present_value = 0;//当前操作的指令
std::map<const koopa_raw_value_t, Reg> value_map;
int global_num = 0;
std::map<const koopa_raw_value_t, std::string> global_values;
int stack_size = 0, stack_top = 0;
bool restore_ra = false;


void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
Reg Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
Reg Visit(const koopa_raw_binary_t &binary);
Reg Visit(const koopa_raw_integer_t &integer);
Reg Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_store_t &store);


int find_reg(int stat);
int cal_size(const koopa_raw_type_t &ty);

void Visit(const koopa_raw_program_t &program)
{
    Visit(program.values);
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice)
{
    //std::cout<<"slice len"<<slice.len<<std::endl;
    for (size_t i = 0; i < slice.len; i++)
    {
        auto ptr = slice.buffer[i];
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
        }
    }
}

void Visit(const koopa_raw_function_t &func)
{
    if(func->bbs.len==0) return;
    std::cout<<" .text"<<std::endl;
    std::cout<<" .globl "<<func->name+1<<std::endl;
    std::cout<<func->name+1<<":"<<std::endl;
    assert(stack_size == 0); assert(stack_top == 0);
    for (size_t i = 0; i < func->bbs.len; i++)
    {
        auto ptr = func->bbs.buffer[i];
        koopa_raw_basic_block_t bb = reinterpret_cast<koopa_raw_basic_block_t>(ptr);
        for (size_t j = 0; j < bb->insts.len; j++)
        {
            ptr = bb->insts.buffer[j];
            koopa_raw_value_t inst = reinterpret_cast<koopa_raw_value_t>(ptr);
            if (inst->ty->tag != KOOPA_RTT_UNIT)
            {
                if (inst->kind.tag == KOOPA_RVT_ALLOC)
                    stack_size += cal_size(inst->ty->data.pointer.base);
                else stack_size += 4;
            }
            //if (inst->kind.tag == KOOPA_RVT_CALL)
            //{
                //restore_ra = true;
                //int arg_num = inst->kind.data.call.args.len;
                //if (arg_num > max_arg_num)max_arg_num = arg_num;
            //}
        }
        stack_size = ceil(stack_size / 16.0) * 16;
        if (stack_size > 0 && stack_size <= 2048)
            std::cout << " addi  sp, sp, -" << stack_size << std::endl;
        else if (stack_size > 2048)
        {
            std::cout << " li    s11, -" << stack_size << std::endl;
            std::cout << " add   sp, sp, s11" << std::endl;
        }
    }
    Visit(func->bbs);
    stack_size=stack_top=0;
    for (int i = 0; i < 16; i++)reg_stats[i] = 0;
    value_map.clear();
    std::cout << std::endl;
}

void Visit(const koopa_raw_basic_block_t &bb)
{
    //std::cout<<bb->name+1<<":"<<std::endl;
    Visit(bb->insts);
}

Reg Visit(const koopa_raw_value_t &value)
{
    koopa_raw_value_t old_value = present_value;
    present_value = value; 

    if(value_map.count(value))//如果当前指令已被处理，则直接返回它所在的寄存器
    {
        if(value_map[value].reg_name==-1)//有可能该指令被转移到栈中
        {
            int reg_name = find_reg(1);//重新找一个可用寄存器，将值转移到新寄存器里
            value_map[value].reg_name = reg_name;
            int reg_offset = value_map[value].reg_offset;
            if (reg_offset >= -2048 && reg_offset <= 2047)
                std::cout << " lw    " << reg_names[reg_name] << ", " << reg_offset << "(sp)" << std::endl;
            else
            {
                std::cout << " li    s11, " << reg_offset << std::endl;
                std::cout << " add   s11, sp, s11" << std::endl;
                std::cout << " lw    " << reg_names[reg_name] << ", (s11)" << std::endl;
            }
        }
        present_value=old_value;
        return value_map[value];
    }

    const auto &kind=value->kind;
    struct Reg result_var = {-1, -1};
    switch(kind.tag)
    {
        case KOOPA_RVT_RETURN:
            Visit(kind.data.ret);
            break;
        case KOOPA_RVT_BINARY:
            result_var=Visit(kind.data.binary);
            value_map[value]=result_var;
            assert(result_var.reg_name>=0);//该条指令当前一定在某个寄存器中，不可能在栈中
            break;
        case KOOPA_RVT_INTEGER:
            result_var=Visit(kind.data.integer);
            break;
        case KOOPA_RVT_ALLOC:
            result_var.reg_offset = stack_top;
            assert(value->ty->tag == KOOPA_RTT_POINTER);
            stack_top += cal_size(value->ty->data.pointer.base);
            value_map[value] = result_var;
            break;
        case KOOPA_RVT_LOAD:
            result_var = Visit(kind.data.load);
            value_map[value] = result_var;
            assert(result_var.reg_name >= 0);
            break;
        case KOOPA_RVT_STORE:
            Visit(kind.data.store);
            break;
        default:
            ;
    }
    present_value = old_value;
    return result_var;
}

void Visit(const koopa_raw_return_t &ret)
{
    koopa_raw_value_t ret_value=ret.value;
    if (ret_value)
    {
        struct Reg result_var = Visit(ret_value);
        assert(result_var.reg_name >= 0);
        if (result_var.reg_name != 7)
            std::cout << " mv    a0, " << reg_names[result_var.reg_name] <<
                std::endl;
    }
    std::cout << " ret" << std::endl;
}

Reg Visit(const koopa_raw_binary_t &binary)
{
    struct Reg left_val = Visit(binary.lhs);//找到运算左操作数所在寄存器
    int left_reg = left_val.reg_name;
    int old_stat = reg_stats[left_reg];
    reg_stats[left_reg] = 2;//防止右操作数选择左操作数所在寄存器
    struct Reg right_val = Visit(binary.rhs);//找到运算右操作数所在寄存器
    int right_reg = right_val.reg_name;
    reg_stats[left_reg] = old_stat;
    old_stat = reg_stats[right_reg];
    reg_stats[right_reg] = 2;//防止结果保存在右操作数所在寄存器
    struct Reg result_var = {find_reg(1), -1};
    reg_stats[right_reg] = old_stat;
    std::string left_name = reg_names[left_reg];
    std::string right_name = reg_names[right_reg];
    std::string result_name = reg_names[result_var.reg_name];
    switch(binary.op)
    {
        case 1://eq
            std::cout << " xor   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            std::cout << " seqz  " << result_name << ", " << result_name << std::endl;
            break;
        case 6://add
            std::cout << " add   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 7://sub
            std::cout << " sub   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 0://ne
            std::cout << " xor   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            std::cout << " snez  " << result_name << ", " << result_name << std::endl;
            break;
        case 2://gt
            std::cout << " sgt   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 3://lt
            std::cout << " slt   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 4://ge
            std::cout << " slt   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            std::cout << " xori  " << result_name << ", " << result_name << ", 1" << std::endl;
            break;
        case 5://le
            std::cout << " sgt   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            std::cout << " xori  " << result_name << ", " << result_name << ", 1" << std::endl;
            break;
        case 8://mul
            std::cout << " mul   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 9://div
            std::cout << " div   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 10://mod
            std::cout << " rem   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 11://and
            std::cout << " and   " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        case 12://or
            std::cout << " or    " << result_name << ", " << left_name << ", " << right_name << std::endl;
            break;
        default:
            assert(false);
    }
    return result_var;
}

Reg Visit(const koopa_raw_integer_t &integer)
{
    int32_t int_val = integer.value;
    struct Reg result_var = {-1, -1};
    if (int_val == 0) { result_var.reg_name = 15; return result_var; }
    result_var.reg_name = find_reg(0);
    std::cout << " li    " << reg_names[result_var.reg_name] << ", " <<int_val << std::endl;
    return result_var;
}

Reg Visit(const koopa_raw_load_t &load)
{
    koopa_raw_value_t src = load.src;
    
    if (value_map[src].reg_name >= 0) return value_map[src];
    int reg_name = find_reg(1), reg_offset = value_map[src].reg_offset;
    struct Reg result_var = {reg_name, reg_offset};
    if (reg_offset >= -2048 && reg_offset <= 2047)
        std::cout << " lw    " << reg_names[reg_name] << ", " << reg_offset << "(sp)" << std::endl;
    else
    {
        std::cout << " li    s11, " << reg_offset << std::endl;
        std::cout << " add   s11, s11, sp" << std::endl;
        std::cout << " lw    " << reg_names[reg_name] << ", (s11)" << std::endl;
    }
    return result_var;
}

void Visit(const koopa_raw_store_t &store)
{
    struct Reg value = Visit(store.value);
    koopa_raw_value_t dest = store.dest;
    assert(value.reg_name >= 0);
    assert(value_map.count(dest));
    if (value_map[dest].reg_offset == -1)
    {
        value_map[dest].reg_offset = stack_top;
        stack_top += 4;
    }
    else //清空过期的值
        for (int i = 0; i < 16; i++)
            if (i == value.reg_name) continue;
            else if (reg_stats[i] > 0 && value_map[registers[i]].reg_offset == value_map[dest].reg_offset)
            {
                reg_stats[i] = 0;  
                value_map[registers[i]].reg_name = value.reg_name;
            }
    int reg_name = value.reg_name, reg_offset = value_map[dest].reg_offset;
    if (reg_offset >= -2048 && reg_offset <= 2047)
        std::cout << " sw    " << reg_names[reg_name] << ", " << reg_offset << "(sp)" << std::endl;
    else
    {
        std::cout << " li    s11, " << reg_offset << std::endl;
        std::cout << " add   s11, s11, sp" << std::endl;
        std::cout << " sw    " << reg_names[reg_name] << ", (s11)" << std::endl;
    }
}

int find_reg(int stat)
{
    for (int i = 0; i < 15; i++)
        if (reg_stats[i] == 0)
        {
            registers[i] = present_value;
            reg_stats[i] = stat;
            return i;
        }
    for (int i = 0; i < 15; i++)
    {
        if (reg_stats[i] == 1)
        {
            value_map[registers[i]].reg_name = -1;//将该寄存器先前对应指令的标志改为-1.意味值保存在栈中
            int offset = value_map[registers[i]].reg_offset;
            if (offset == -1)
            {
                offset = stack_top;
                stack_top += 4;
                value_map[registers[i]].reg_offset = offset;
            }
            if (offset >= -2048 && offset <= 2047)
                std::cout << " sw    " << reg_names[i] << ", " << offset <<
                    "(sp)" << std::endl;
            else
            {
                std::cout << " li    s11, " << offset << std::endl;
                std::cout << " add   s11, s11, sp" << std::endl;
                std::cout << " sw    " << reg_names[i] << ", (s11)" <<
                    std::endl;
            }
            registers[i] = present_value;
            reg_stats[i] = stat;
            return i;
        }
    }
    assert(false);
    return -1;
}

int cal_size(const koopa_raw_type_t &ty)
{
    assert(ty->tag != KOOPA_RTT_UNIT);
    if (ty->tag == KOOPA_RTT_ARRAY)
    {
        int prev = cal_size(ty->data.array.base);
        int len = ty->data.array.len;
        return len * prev;
    }
    return 4;
}

void parse_string(const char* str)
{
  // 解析字符串 str, 得到 Koopa IR 程序
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  
  Visit(raw);
  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program builder 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}
