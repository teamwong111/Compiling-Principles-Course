#include "IntermediateCode.h"

//形成四元式
void IntermediateCode::emit(string op, string src1, string src2, string des) 
{
	code.push_back(Quaternion{op, src1, src2, des});
}

//back_patch
void IntermediateCode::back_patch(list<int>nextList, int quad) 
{
	for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++) 
	{
		code[*iter].des = to_string(quad);
	}
}

//nextquad
int IntermediateCode::get_nextquad() 
{
	return code.size();
}

//输出中间代码
void IntermediateCode::output_code(const char* fileName) 
{
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) 
	{
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	for (int i = 0; i < code.size();++i)
	{
		fout << i << "( " << code[i].op << " , " << code[i].src1 << " , " << code[i].src2 << " , " << code[i].des << " )" << endl;
	}
	fout.close();
}
