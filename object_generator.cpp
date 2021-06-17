#include "object_generator.h"

bool is_var(string name) {
	return isalpha(name[0]);
}

bool is_num(string name) {
	return isdigit(name[0]);
}

bool is_control(string op) {
	if (op[0] == 'j' || op == "call" || op == "return" || op == "get") {
		return true;
	}
	return false;
}

ObjectGenerator::ObjectGenerator() {}

void ObjectGenerator::block_addinfo(map<string, vector<Block> > funcBlocks) {
	for (auto& fb : funcBlocks) {
		vector<Block> base_blocks = fb.second;
		vector<set<string> >func_block_inl, func_block_outl, func_block_def;

		//数据流方程, 确定DEF，USE
		//初始化符号表
		vector<map<string, Info> > func_block_table;	
		for (unsigned int j = 0; j < base_blocks.size(); ++j) {
			set<string> base_block_def, base_block_use;
			map<string, Info> base_block_table;
			for(unsigned int k = 0; k < base_blocks[j].codes.size(); ++k) {
				if (base_blocks[j].codes[k].op != "j" && base_blocks[j].codes[k].op != "call") {
					if (is_var(base_blocks[j].codes[k].src1)) { 
						if(base_block_def.count(base_blocks[j].codes[k].src1) == 0) {//如果源操作数1还没有被定值
							base_block_use.insert(base_blocks[j].codes[k].src1);
						}
						base_block_table[base_blocks[j].codes[k].src1] = Info{ -1,false };
					}
					if (is_var(base_blocks[j].codes[k].src2)) {
						if (base_block_def.count(base_blocks[j].codes[k].src2) == 0) {//如果源操作数2还没有被定值
							base_block_use.insert(base_blocks[j].codes[k].src2);
						}
						base_block_table[base_blocks[j].codes[k].src2] = Info{ -1,false };
					}
					if (base_blocks[j].codes[k].op[0] != 'j' && is_var(base_blocks[j].codes[k].des)) {
						if (base_block_use.count(base_blocks[j].codes[k].des) == 0) {//如果目的操作数还没有被引用
							base_block_def.insert(base_blocks[j].codes[k].des);
						}
						base_block_table[base_blocks[j].codes[k].des] = Info{ -1,false };
					}
				}
			}
			func_block_inl.push_back(base_block_use);
			func_block_def.push_back(base_block_def);
			func_block_outl.push_back(set<string>());
			func_block_table.push_back(base_block_table);
		}

		//确定INL，OUTL
		bool change = true;
		while (change) {
			change = false;
			for (unsigned int j = 0;j < base_blocks.size(); ++j) {
				int next[2] = { base_blocks[j].next1, base_blocks[j].next2 };
				for(int k = 0; k < 2; ++k) {
					if(next[k]!= -1) {
						for (auto& l_var : func_block_inl[next[k]]) {
							if (func_block_outl[j].insert(l_var).second == true) {
								if (func_block_def[j].count(l_var) == 0) {
									func_block_inl[j].insert(l_var);
								}
								change = true;
							}
						}
					}
				}
			}
		}
		func_outls[fb.first] = func_block_outl;
		func_inls[fb.first] = func_block_inl;

		for (unsigned int j = 0;j < func_block_outl.size(); ++j) {//遍历每个基本块的活跃变量表
			for (auto& l_var : func_block_outl[j]) {//遍历活跃变量表中的变量
				func_block_table[j][l_var] = Info{ -1, true };
			}
		}

		for (unsigned int j = 0; j < base_blocks.size(); ++j) {
			for(int k = base_blocks[j].codes.size() - 1; k >= 0; --k) {//逆序遍历基本块中的代码
				if (base_blocks[j].codes[k].op != "j" && base_blocks[j].codes[k].op != "call") {
					if (is_var(base_blocks[j].codes[k].src1)) {
						base_blocks[j].codes[k].info[0] = func_block_table[j][base_blocks[j].codes[k].src1];
						func_block_table[j][base_blocks[j].codes[k].src1] = Info{ k, true };
					}
					if (is_var(base_blocks[j].codes[k].src2)) {
						base_blocks[j].codes[k].info[1] = func_block_table[j][base_blocks[j].codes[k].src2];
						func_block_table[j][base_blocks[j].codes[k].src2] = Info{ k, true };
					}
					if (base_blocks[j].codes[k].op[0] != 'j' && is_var(base_blocks[j].codes[k].des)) { // 不是跳转指令才关心左值
						base_blocks[j].codes[k].info[2] = func_block_table[j][base_blocks[j].codes[k].des];
						func_block_table[j][base_blocks[j].codes[k].des] = Info{ -1,false };
					}
				}
			}
		}
		func_blocks[fb.first] = base_blocks;
	}
}

void ObjectGenerator::generate_codes() {
	object_codes.push_back("lui $sp,0x1001");
	object_codes.push_back("j main");
	for (auto& fb : func_blocks) {
		var_storepos.clear();
		present_func = fb.first;
		for (unsigned int i = 0; i < fb.second.size(); ++i) {
			present_block = fb.second[i];
			present_block_index = i;
			generate_base_block();
		}
	}
	object_codes.push_back("end:");
}

void ObjectGenerator::generate_base_block() {
	// 初始化
	Avalue.clear();
	Rvalue.clear();
	free_register.clear();
	for (auto& i_var : func_inls[present_func][present_block_index]) {
		Avalue[i_var].insert(i_var);
	}
	for (int i = 0; i <= 7; i++) {
		free_register.push_back(string("$s") + to_string(i));
	}

	// 操作1
	object_codes.push_back(present_block.name + ":");
	if (present_block_index == 0) {
		if (present_func == "main") {
			present_top = 8;
		}
		else {
			object_codes.push_back("sw $ra 4($sp)");//把返回地址压栈
			present_top = 8;
		}
	}

	// 
	int arg_num = 0;//par的实参个数
	list<pair<string, bool> > par_list;//函数调用用到的实参集list<实参名,是否活跃>
	for (unsigned int i = 0; i < present_block.codes.size(); ++i) {
		present_quaternion = present_block.codes[i];
		present_quaternion_index = i;
		//如果是基本块的最后一条语句
		if (i == present_block.codes.size() - 1) {
			//控制语句，则先将出口活跃变量保存，再进行跳转(j,call,return)
			if (is_control(present_block.codes[i].op)) {
				store_oul_var(func_outls[present_func][present_block_index]);
				generate_quaternion(arg_num, par_list);
			}
			//如果最后一条语句是赋值语句，则先计算，再将出口活跃变量保存
			else {
				generate_quaternion(arg_num, par_list);
				store_oul_var(func_outls[present_func][present_block_index]);
			}
		}
		else {
			generate_quaternion(arg_num, par_list);
		}
	}
}

void ObjectGenerator::generate_quaternion(int &arg_num, list<pair<string, bool> > &par_list) {
	if (present_quaternion.op[0] != 'j' && present_quaternion.op != "call") {
		if (is_var(present_quaternion.src1) && Avalue[present_quaternion.src1].empty()) {
			cout << (string("变量") + present_quaternion.src1 + "在引用前未赋值");
		}
		if (is_var(present_quaternion.src2) && Avalue[present_quaternion.src2].empty()) {
			cout << (string("变量") + present_quaternion.src2 + "在引用前未赋值");
		}
	}
	
	if (present_quaternion.op == "j") {
		object_codes.push_back(present_quaternion.op + " " + present_quaternion.des);
	}
	else if (present_quaternion.op == "par") {
		par_list.push_back(pair<string, bool>(present_quaternion.src1, present_quaternion.info[0].active));
	}
	else if (present_quaternion.op == "call") {
		// 将参数压栈
		for (auto& arg : par_list) {
			string pos = allocate_src_reg(arg.first);
			object_codes.push_back(string("sw ") + pos + " " + to_string(present_top + 4 * (++arg_num + 1)) + "($sp)");
			if (!arg.second) {
				release_var(arg.first);
			}
		}
		// 更新$sp
		object_codes.push_back(string("sw $sp ") + to_string(present_top) + "($sp)");
		object_codes.push_back(string("addi $sp $sp ") + to_string(present_top));

		// 跳转到对应函数
		object_codes.push_back(string("jal ") + present_quaternion.src1);

		// 恢复现场
		object_codes.push_back(string("lw $sp 0($sp)"));
	}
	else if (present_quaternion.op == "return") {
		if (is_num(present_quaternion.src1)) {//返回值为数字
			object_codes.push_back("addi $v0 $zero " + present_quaternion.src1);
		}
		else if (is_var(present_quaternion.src1)) {//返回值为变量
			set<string>::iterator piter = Avalue[present_quaternion.src1].begin();
			if ((*piter)[0] == '$') {
				object_codes.push_back(string("add $v0 $zero ") + *piter);
			}
			else {
				object_codes.push_back(string("lw $v0 ") + to_string(var_storepos[*piter]) + "($sp)");
			}
		}
		if (present_func == "main") {
			object_codes.push_back("j end");
		}
		else {
			object_codes.push_back("lw $ra 4($sp)");
			object_codes.push_back("jr $ra");
		}
	}
	else if (present_quaternion.op == "get") {
		var_storepos[present_quaternion.des] = present_top;
		present_top += 4;
		Avalue[present_quaternion.des].insert(present_quaternion.des);
	}
	else if (present_quaternion.op == "=") {
		string src1_pos;
		if (present_quaternion.src1 == "@RETURN_PLACE") {
			src1_pos = "$v0";
		}
		else {
			src1_pos = allocate_src_reg(present_quaternion.src1);
		}
		Rvalue[src1_pos].insert(present_quaternion.des);
		Avalue[present_quaternion.des].insert(src1_pos);
	}
	else {
		string pos1 = allocate_src_reg(present_quaternion.src1);
		string pos2 = allocate_src_reg(present_quaternion.src2);
		if (present_quaternion.op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
			object_codes.push_back(irop_ocop[present_quaternion.op] + pos1 + " " + pos2 + " " + present_quaternion.des);
		}
		else {//+ - * /
			string des_pos = allocate_des_reg();
			if(present_quaternion.op != "/") {
				object_codes.push_back(irop_ocop[present_quaternion.op] + des_pos + " " + pos1 + " " + pos2);
			}
			else {
				object_codes.push_back(string("div ") + pos1 + " " + pos2);
				object_codes.push_back(string("mflo ") + des_pos);
			}
		}
		if (!present_quaternion.info[0].active) {
			release_var(present_quaternion.src1);
		}
		if (!present_quaternion.info[1].active) {
			release_var(present_quaternion.src2);
		}
	}
}

void ObjectGenerator::store_oul_var(set<string> outl) {
	for (auto& ol : outl) {
		string reg;
		bool in_memory = false;
		for (auto& al : Avalue[ol]) {
			if (al[0] != '$') {//该活跃变量已经存储在内存中
				in_memory = true;
				break;
			}
			else {
				reg = al;
			}
		}
		if (!in_memory) {//如果该活跃变量不在内存中，则将reg中的var变量存入内存
			store_reg_var(reg, ol);
		}
	}
}

void ObjectGenerator::store_reg_var(string reg, string var) {
	if (var_storepos.find(var) != var_storepos.end()) {//如果已经为变量分配好了存储空间
		object_codes.push_back(string("sw ") + reg + " " + to_string(var_storepos[var]) + "($sp)");
	}
	else {
		var_storepos[var] = present_top;
		present_top += 4;
		object_codes.push_back(string("sw ") + reg + " " + to_string(var_storepos[var]) + "($sp)");
	}
	Avalue[var].insert(var);
}

void ObjectGenerator::release_var(string var) {
	for (auto& a_var : Avalue[var]) {
		if (a_var[0] == '$') {
			Rvalue[a_var].erase(var);
			if (Rvalue[a_var].size() == 0 && (a_var)[1] == 's') {
				free_register.push_back(a_var);
			}
		}
	}
	Avalue[var].clear();
}

string ObjectGenerator::allocate_des_reg() {
	//A:=B op C
	if (!is_num(present_quaternion.src1)) {
		for (auto& r_var : Avalue[present_quaternion.src1]) {
			if (r_var[0] == '$' && Rvalue[r_var].size() == 1) {//如果B的现行值在某个寄存器Ri中，RVALUE[Ri]中只包含B
				if (present_quaternion.des == present_quaternion.src1 || !present_quaternion.info[0].active) {//如果A,B是同一标识符或B以后不活跃
					Avalue[present_quaternion.des].insert(r_var);
					Rvalue[r_var].insert(present_quaternion.des);
					return r_var;
				}
			}
		}
	}

	string ret = allocate_src_reg();
	Avalue[present_quaternion.des].insert(ret);
	Rvalue[ret].insert(present_quaternion.des);
	return ret;
}

string ObjectGenerator::allocate_src_reg(string var) {
	if (is_num(var)) {
		string ret = allocate_src_reg();
		object_codes.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	for (auto& r_var : Avalue[var]) {
		if (r_var[0] == '$') {//如果变量已经保存在某个寄存器中
			return r_var;//直接返回该寄存器
		}
	}

	//如果该变量没有在某个寄存器中
	string ret = allocate_src_reg();
	object_codes.push_back(string("lw ") + ret + " " + to_string(var_storepos[var]) + "($sp)");
	Avalue[var].insert(ret);
	Rvalue[ret].insert(var);
	return ret;
}

string ObjectGenerator::allocate_src_reg() {
	//如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	string ret;
	if (free_register.size()) {
		ret = free_register.back();
		free_register.pop_back();
		return ret;
	}

	// 从已分配的寄存器中选取一个Ri为所需要的寄存器R。最好使得Ri满足以下条件：
	// 占用Ri的变量的值也同时存放在该变量的贮存单元中或者在基本块中要在最远的将来才会引用到或不会引用到。

	const int inf = 1000000;
	int max_pos = 0;
	for (auto& iter : Rvalue) {//遍历所有寄存器
		int next_pos = inf;
		for (auto& viter : iter.second) {//遍历寄存器中储存的变量
			bool only_in_reg = true;
			for (auto& aiter : Avalue[viter]) {//遍历变量的存储位置
				if (aiter != iter.first) {//如果变量存储在其他地方
					only_in_reg = false;
					break;
				}
			}
			if (only_in_reg) {
				for (unsigned int l = present_quaternion_index; l < present_block.codes.size(); ++l) {
					if (viter == present_block.codes[l].src1 || viter == present_block.codes[l].src2) {
						next_pos = l - present_quaternion_index;
					}
					else if (viter == present_block.codes[l].des) {
						break;
					}
				}
			}
		}
		if (next_pos == inf) {
			ret = iter.first;
			break;
		}
		else if (next_pos > max_pos) {
			max_pos = next_pos;
			ret = iter.first;
		}
	}

	for (auto& iter : Rvalue[ret]) {//ret的寄存器中保存的变量都将不再存储在ret中
		Avalue[iter].erase(ret);
		if (Avalue[iter].size() == 0) {//如果v只保存当前的寄存器并且之后要使用，则需要存数指令
			bool need_store = true;
			unsigned int j;
			for (j = present_quaternion_index; j < present_block.codes.size(); ++j) {
				if (present_block.codes[j].src1 == iter || present_block.codes[j].src2 == iter) {//如果V在本基本块中被引用
					need_store = true;
					break;
				}
				if (present_block.codes[j].des == iter) {//如果V在本基本块中被赋值
					need_store = false;
					break;
				}
			}
			if (j == present_block.codes.size() - 1) {//如果V在本基本块中未被引用，且也没有被赋值
				if (func_outls[present_func][present_block_index].count(iter) == 1) {//如果此变量是出口之后的活跃变量
					need_store = true;
				}
				else {
					need_store = false;
				}
			}
			if (need_store) {//生成存数指令
				store_reg_var(ret, iter);
			}
		}
	}
	Rvalue[ret].clear();//清空ret寄存器中保存的变量
	return ret;
}

void ObjectGenerator::output_blocks(ostream& out) {
	for (auto& fb : func_blocks) {
		out << "[" << fb.first << "]" << endl;
		for (unsigned int i = 0; i < fb.second.size(); ++i) {
			out << fb.second[i].name << ":" << endl;
			for (unsigned int j = 0; j < fb.second[i].codes.size(); ++j) {
				out << "    ";
				fb.second[i].codes[j].output(out);
				out << endl;
			}
			out << "    " << "next1 = " << fb.second[i].next1 << endl;
			out << "    " << "next2 = " << fb.second[i].next2 << endl;
		}
		cout << endl;
	}
}

void ObjectGenerator::output_object_codes(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	for (unsigned int i = 0; i < object_codes.size(); ++i) {
		fout << object_codes[i] << endl;
	}
	cout << "目标代码生成成功" << endl;
	fout.close();
}

