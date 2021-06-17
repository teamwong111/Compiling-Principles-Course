#include "ir_generator.h"

Quaternion::Quaternion() {}

Quaternion::Quaternion(string op, string src1, string src2, string des) {
    this->op = op;
    this->src1 = src1;
    this->src2 = src2;
    this->des = des;
    this->info[0] = {-1, false};
    this->info[1] = {-1, false};
    this->info[2] = {-1, false};
}

//形成四元式
void IRGenerator::emit(string op, string src1, string src2, string des) {
    code.push_back(Quaternion(op, src1, src2, des));
}

// back_patch
void IRGenerator::back_patch(list<int> nextList, int quad) {
    for (auto& nl : nextList) {
        code[nl].des = to_string(quad);
    }
}

// nextquad
int IRGenerator::get_nextquad() { return code.size(); }

//输出中间代码
void IRGenerator::output_code(const char* fileName) {
    ofstream fout;
    fout.open(fileName);
    if (!fout.is_open()) {
        cerr << "file " << fileName << " open error" << endl;
        return;
    }
    for (unsigned int i = 0; i < code.size(); ++i) {
        fout << i << "( " << code[i].op << " , " << code[i].src1 << " , "
             << code[i].src2 << " , " << code[i].des << " )" << endl;
    }
    cout << "中间代码生成成功" << endl;
    fout.close();
}

IRGenerator::IRGenerator() { name_index = 1; }

string IRGenerator::get_block_name() { return string("Label") + to_string(name_index++); }

void IRGenerator::divide_block(vector<pair<int, string> > func_start) {
    for (unsigned int i = 0; i < func_start.size(); ++i) {
        //记录所有基本块的入口点
        priority_queue<int, vector<int>, greater<int> > baseblock_start;  
        baseblock_start.push(func_start[i].first);
        int funcblock_end = (i == func_start.size() - 1) ? code.size() : func_start[i+1].first;
        baseblock_start.push(funcblock_end); // 把funcblock的结尾加入
        for (int j = func_start[i].first; j != funcblock_end; j++) {
            if (code[j].op == "j") {
                baseblock_start.push(atoi(code[j].des.c_str()));
            } 
			else if (code[j].op == "j=-" || code[j].op == "j!=" || code[j].op == "j>=" || 
			         code[j].op == "j<=" || code[j].op == "j<" || code[j].op == "j>") {
                baseblock_start.push(atoi(code[j].des.c_str()));
                if (j + 1 < funcblock_end) {
                    baseblock_start.push(j + 1);
                }
            } 
			else if ((code[j].op == "return" || code[j].op == "call") && j + 1 < funcblock_end) {
                baseblock_start.push(j + 1);
            }
        }

        //划分基本块
        Block block;
        vector<Block> funcblock;
        map<int, string> start_label;
        map<int, int> start_block;
        bool first_block = true;
        int start = baseblock_start.top();
        baseblock_start.pop();
        while (!baseblock_start.empty()) {
            int end = baseblock_start.top();
            baseblock_start.pop();
            if (end == start) {
                continue;
            }
            for (int j = start; j != end; j++) {
                block.codes.push_back(code[j]);
            }
            if (first_block) {
                block.name = func_start[i].second;
                first_block = false;
            } 
            else {
                block.name = get_block_name();
                start_label[start] = block.name;
            }
            start_block[start] = funcblock.size();
            funcblock.push_back(block);
            start = end;
            block.codes.clear();
        }

        for (unsigned int j =0; j < funcblock.size(); ++j) {
            int codes_size = funcblock[j].codes.size();
            Quaternion lastCode = funcblock[j].codes[codes_size - 1];
            funcblock[j].next2 = -1;
            if (lastCode.op == "j") {
                funcblock[j].next1 = start_block[atoi(lastCode.des.c_str())];
                funcblock[j].codes[codes_size - 1].des = start_label[atoi(lastCode.des.c_str())];
            } 
			else if (lastCode.op == "j=-" || lastCode.op == "j!=" || lastCode.op == "j>=" || 
					 lastCode.op == "j<=" || lastCode.op == "j<" || lastCode.op == "j>") {
                funcblock[j].next1 = j + 1;
                funcblock[j].next2 = (funcblock[j].next1 == start_block[atoi(lastCode.des.c_str())]) ? -1 : start_block[atoi(lastCode.des.c_str())];
                funcblock[j].codes[codes_size - 1].des = start_label[atoi(lastCode.des.c_str())];
            } 
			else if (lastCode.op == "return") {
                funcblock[j].next1 = -1;
            } 
			else {
                funcblock[j].next1 = j + 1;
            }
        }
        func_blocks[func_start[i].second] = funcblock;
    }
}

map<string, vector<Block> > IRGenerator::get_func_blocks() {
	return func_blocks;
}

void Quaternion::output(ostream& out) {
	out << "(" << op << "," << src1 << "," << src2 << "," << des << ")";
	for(int i=0;i<3;++i) {
		out << "(";
		if (info[i].pending == -1)
			out << "^";
		else
			out << info[i].pending;
		out << ",";
		if (info[i].active)
			out << "y";
		else
			out << "^";
		out << ")";
	}
}