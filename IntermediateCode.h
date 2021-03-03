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
using namespace std;

struct Quaternion 
{
	string op;//操作数
	string src1;//参数一
	string src2;//参数二
	string des;//地址
};

class IntermediateCode 
{
public:	
	vector<Quaternion> code;//中间代码
	void emit(string, string, string, string);//形成四元式
	void back_patch(list<int>, int);//back_patch
	void output_code(const char*);//输出中间代码
	int get_nextquad();//nextquad
};
