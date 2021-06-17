#include "ir_generator.h"

//当前状态和读取符号
typedef pair<int, string> Goto;

//动作
enum Act { reduct, shift, accept, error, _goto };

//数据类型（int/void）
enum Datetype { D_VOID, D_INT };

//产生式
struct Production
{
	int id;
	string left;
	vector<string>right;
};

//项目
struct Item
{
	unsigned int pid;
	unsigned int pointidx;//.的位置
	friend bool operator ==(const Item& one, const Item& other);
	friend bool operator <(const Item& one, const Item& other);
};

//dfa
struct DFA
{
	vector<set<Item> > states;
	map<Goto, unsigned int> goTo;
};

//状态转移
struct Move
{
	Act act;
	unsigned int nextstate;
};

//变量
struct Var 
{
	string name;
	Datetype type;
	int level;
};

//函数
struct Func 
{
	string name;
	Datetype returntype;
	list<Datetype> paramtype;
	int startaddr;
};

//符号
class E
{
public:
	string name;//ID
	string real_name;//名字
	list<Datetype> plist;//参数列表
	list<int> nextList;
	list<int> falseList;
	list<string> alist;
	int quad;
	string offset; //数组偏移
	E(string name);
	E(string name, string real_name);
};

//语法分析和词法分析类
class Parser
{
private:
	int now_level;
	vector<Production> productions;	//产生式
	map<string, set<string> >first;	//first集
	map<string, set<string> >follow;//follow集
	DFA dfa;						//dfa
	map<Goto, Move> action_goto;	//action_goto表
	stack<string> symbol_stack;		//符号栈
	stack<int> state_stack;			//状态栈
	int temp_var;					//临时变量
	stack<E> symbol_stack_e;		//语义分析符号栈
	IRGenerator code;				//中间代码
	vector<Var> varTable;			//变量表
	vector<Func> funcTable;			//函数表
public:
	Parser();									//构造函数
	bool is_terminal(string);					//是否是终结符
	void read_grammars(const char*);			//读取文法
	void get_first();							//获取first集
	void get_follow();							//获取follow集
	set<Item> get_closure(Item);				//获取闭包
	void get_itemsets();						//获取项目集规范族
	void get_action_goto();						//获取action_goto
	void analyse(const char*);					//语法分析
	void output_error(string);					//输出错误
	void output_dfa(const char*);				//输出DFA
	void output_action_goto(const char*);		//输出action_goto表
	string get_newtemp();						//新的临时变量
	E pop_symbol();								//pop & get top
	void push_symbol(E);						//push
	void analyse_e(const char*, const char*);	//语义分析
	Func search_func(string);					//寻找函数
	Var search_var(string);						//寻找变量
	list<int> merge(list<int>&, list<int>&);	//合并
	void output_stack(ofstream& out);			//输出符号栈、状态栈
	void output_production(const char*);		//输出产生式
	vector<pair<int, string> > get_funcstart(); //获取函数位置
	IRGenerator get_ir();						//获取中间代码
	void get_result(const char*, const char*, const char*);	//获取中间代码
};
