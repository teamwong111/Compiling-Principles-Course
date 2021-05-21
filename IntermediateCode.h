#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <string>
#include <queue>
using namespace std;

struct Info {
	int pending;//待用信息
	bool active;//活跃信息
};

class Quaternion 
{
public:
	string op;//操作数
	string src1;//参数一
	string src2;//参数二
	string des;//地址
	Info info[3];
	Quaternion();
	Quaternion(string op, string src1, string src2, string des);
	void output(ostream& out);
};

struct Block {
	string name;//基本块的名称
	vector<Quaternion> codes;//基本块中的四元式
	int next1;//基本块的下一连接块
	int next2;//基本块的下一连接块
};


class IntermediateCode 
{
private:
	int name_index;	
	vector<Quaternion> code;//中间代码
	map<string, vector<Block> > func_blocks;
public:
	IntermediateCode();
	void emit(string, string, string, string);//形成四元式
	void back_patch(list<int>, int);//back_patch
	void output_code(const char*);//输出中间代码
	int get_nextquad();//nextquad
	string get_block_name();
	void divide_block(vector<pair<int, string> > func_enter);//划分基本快
	map<string, vector<Block> > get_func_blocks();
};
