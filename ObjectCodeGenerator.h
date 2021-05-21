#pragma once
#include "IntermediateCode.h"

//保存临时常数 t0 t1寄存器
//保存函数的返回值 v0寄存器
class ObjectCodeGenerator {
private:
	map<string, string> irop_ocop = { { "j>=", "bge " }, { "j>", "bgt " }, { "j==", "beq " }, 
									  { "j!=", "bne " }, { "j<", "blt " }, { "j<=", "ble " },
									  { "+", "add " }, { "-", "sub " }, { "*", "mul " }, }; // IR的OP到目标代码OP的映射

	map<string,vector<Block> > func_blocks;
	map<string, vector<set<string> > > func_outls;//各函数块中基本块的出口活跃变量集
	map<string, vector<set<string> > > func_inls;//各函数块中基本块的入口活跃变量集
	map<string, set<string> > Avalue;
	map<string, set<string> > Rvalue;
	vector<string>free_register;//空闲的寄存器编号
	map<string, int>var_storepos;//各变量的存储位置
	int present_top;//当前栈顶
	string present_func;//当前分析的函数
	Block present_block;//当前分析的基本块
	int present_block_index;
	Quaternion present_quaternion;//当前分析的四元式
	int present_quaternion_index;
	vector<string>object_codes;
public:
	ObjectCodeGenerator();
	void store_reg_var(string reg, string var);
	void store_oul_var(set<string> outl);
	void release_var(string var);
	string allocate_des_reg();
	string allocate_src_reg();
	string allocate_src_reg(string var);

	void block_addinfo(map<string, vector<Block> > funcBlocks);	
	
	void generate_codes();
	void generate_base_block();
	void generate_quaternion(int &arg_num, list<pair<string, bool> > &par_list);
	
	void output_object_codes(const char* fileName);
	void output_blocks(ostream& out);
};