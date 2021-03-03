#include "parser.h"

E::E(string name)
{
	this->name = name;
}

E::E(string name, string real_name)
{
	this->name = name;
	this->real_name = real_name;
}

bool operator < (const Item& one, const Item& other)
{
	return pair<int, int>(one.pid, one.pointidx) < pair<int, int>(other.pid, other.pointidx);
}
bool operator ==(const Item& one, const Item& other)
{
	return one.pid == other.pid && one.pointidx == other.pointidx;
}

//构造函数，执行相关函数
Parser::Parser(const char* filename)
{
	now_level = 0;
	temp_var = 0;
	read_grammars(filename);
	output_production("produtions.txt");
	get_first();
	get_follow();
	get_itemsets();
	get_action_goto();
}

//是否是终结符
bool Parser::is_terminal(string s)
{
	if (s == "int" || s == "void" || s == "if" || s == "while" || s == "else" || s == "return" ||
		s == "+" || s == "-" || s == "*" || s == "/" || s == "=" || s == "==" || s == ">" || s == "<" || s == "!=" || s == ">=" || s == "<=" ||
		s == ";" || s == "," || s == "(" || s == ")" || s == "{" || s == "}" || s == "ID" || s == "NUM")
		return true;
	return false;
}

//读取文法
void Parser::read_grammars(const char* filename)
{
	ifstream fin;
	fin.open(filename, ios::in);
	if (!fin.is_open()) output_error("文件" + string(filename) + "打开失败");
	int index = 0;
	char buf[1024];
	while (fin >> buf)
	{
		Production p;
		p.id = index++;//产生式序号
		p.left = string(buf);//产生式左部
		fin >> buf;//产生式中间
		fin.getline(buf, 1024);
		stringstream sstream(buf);
		string temp;
		while (sstream >> temp)
		{
			p.right.push_back(temp);//产生式右部
		}
		productions.push_back(p);//插入产生式
	}
}

//获取first集
void Parser::get_first()
{
	bool changeFlag = true;//first集改变标志
	while (changeFlag)
	{
		changeFlag = false;
		for (vector<Production>::iterator iter = productions.begin(); iter != productions.end(); iter++)//遍历每一个产生式
		{
			vector<string>::iterator symIter;
			for (symIter = iter->right.begin(); symIter != iter->right.end(); symIter++)//依次遍历产生式右部的所有符号
			{
				if (is_terminal(*symIter))//这个右部符号是终结符
				{
					if (first.count(iter->left) == 0)
					{
						first[iter->left] = set<string>();
					}
					if (first[iter->left].insert(*symIter).second == true)//左部符号的first集不包含该右部符号
					{
						changeFlag = true;
					}
					break;
				}
				else//当前右部符号是非终结符
				{
					bool continueFlag = false;//是否继续读取下一个右部符号的first集
					set<string>::iterator firstIter;
					for (firstIter = first[*symIter].begin(); firstIter != first[*symIter].end(); firstIter++)//遍历该右部符号的first集
					{
						if (*firstIter == "EMPTY")//右部符号的first集中的元素包含EMPTY
						{
							continueFlag = true;
						}
						else if (first[iter->left].find(*firstIter) == first[iter->left].end())//右部符号的first集中的元素不在左部符号first集中
						{
							if (first.count(iter->left) == 0)
							{
								first[iter->left] = set<string>();
							}
							first[iter->left].insert(*firstIter);
							changeFlag = true;
						}
					}
					if (!continueFlag)
					{
						break;
					}
				}
			}
			if (symIter == iter->right.end())//遍历右部符号到了末尾,则EMPTY在其first集中
			{
				if (first.count(iter->left) == 0)
				{
					first[iter->left] = set<string>();
				}
				if (first[iter->left].insert("EMPTY").second == true)
				{
					changeFlag = true;
				}
			}
		}

	}
}

//获取follow集
void Parser::get_follow()
{
	follow[productions[0].left] = set<string>();//将#放入起始符号的FOLLOW集中
	follow[productions[0].left].insert("#");
	bool changeFlag = true;
	while (changeFlag)
	{
		changeFlag = false;
		for (vector<Production>::iterator proIter = productions.begin(); proIter != productions.end(); proIter++)//遍历每一个产生式
		{
			for (vector<string>::iterator symIter = proIter->right.begin(); symIter != proIter->right.end(); symIter++)//遍历产生式右部的每个符号
			{
				vector<string>::iterator nextSymIter;
				for (nextSymIter = symIter + 1; nextSymIter != proIter->right.end(); nextSymIter++)//遍历产生式右部该符号之后的符号
				{
					string nextSym = *nextSymIter;
					bool nextFlag = false;
					if (is_terminal(nextSym))//如果之后的符号是终结符
					{
						if (follow.count(*symIter) == 0)
						{
							follow[*symIter] = set<string>();
						}
						if (follow[*symIter].insert(nextSym).second == true)//如果成功插入新值
						{
							changeFlag = true;
						}
					}
					else
					{
						for (set<string>::iterator fIter = first[nextSym].begin(); fIter != first[nextSym].end(); fIter++)//遍历之后符号的first集
						{
							if (*fIter == "EMPTY")//如果当前符号first集中有空串
							{
								nextFlag = true;
							}
							else
							{
								if (follow.count(*symIter) == 0)
								{
									follow[*symIter] = set<string>();
								}								
								if (follow[*symIter].insert(*fIter).second == true)//如果成功插入新值
								{
									changeFlag = true;
								}
							}
						}
					}					
					if (!nextFlag)//如果当前符号first集中没有空串
					{
						break;
					}

				}	
				if (nextSymIter == proIter->right.end())//如果遍历到了结尾,将左部符号的FOLLOW集加入其FOLLOW集中
				{
					for (set<string>::iterator followIter = follow[proIter->left].begin(); followIter != follow[proIter->left].end(); followIter++)//遍历左部符号的FOLLOW集
					{
						if (follow.count(*symIter) == 0)
						{
							follow[*symIter] = set<string>();
						}
						if (follow[*symIter].insert(*followIter).second == true)//如果该FOLLOW集是新值
						{
							changeFlag = true;
						}
					}
				}
			}
		}
	}
}

//获取闭包
set<Item> Parser::get_closure(Item item)
{
	set<Item> item_temp;
	if (productions[item.pid].right.size() == item.pointidx ||
		is_terminal(productions[item.pid].right[item.pointidx]))//.在项目产生式的最右边(规约项目),	或者.的右边是终结符
	{
		item_temp.insert(item);
	}
	else//.的右边是非终结符
	{
		item_temp.insert(item);
		for (int i = 0; i < productions.size(); ++i)
		{
			if (productions[i].left == productions[item.pid].right[item.pointidx])//产生式的左部 == .右边的非终结符
			{
				set<Item> item_nexttemp = get_closure(Item{ i, 0 });
				for (set<Item>::iterator iter = item_nexttemp.begin(); iter != item_nexttemp.end(); iter++)
				{
					item_temp.insert(*iter);
				}
			}
		}
	}
	return item_temp;
}

//获取项目集规范族
void Parser::get_itemsets()
{
	dfa.states.push_back(get_closure(Item{0 ,0}));//第一个状态
	for (int i = 0; i < dfa.states.size(); ++i)//遍历每一个状态
	{
		for (set<Item>::iterator iter = dfa.states[i].begin(); iter != dfa.states[i].end(); iter++)//遍历状态的每一个项目
		{
			if (productions[iter->pid].right.size() <= iter->pointidx) continue;//后边没有符号
			string next_symbol = productions[iter->pid].right[iter->pointidx];//获取.之后的符号
			if (dfa.goTo.count(Goto(i, next_symbol)) == 1) continue;//已经有该转移
			Item new_item = { iter->pid,iter->pointidx + 1 };
			set<Item> new_setitem = get_closure(new_item);//新产生的状态
			set<Item>::iterator next_iter = iter;
			next_iter++;//指向当前状态项目的下一个项目
			for (; next_iter != dfa.states[i].end(); ++next_iter)
			{
				if (productions[next_iter->pid].right.size() > next_iter->pointidx &&
					productions[next_iter->pid].right[next_iter->pointidx] == next_symbol)//如果是移进项目，且移进next_ssymbol
				{
					Item temp_item = { next_iter->pid, next_iter->pointidx + 1 };
					set<Item> temp_setitem = get_closure(temp_item);
					new_setitem.insert(temp_setitem.begin(), temp_setitem.end());
				}
			}
			vector<set<Item>>::iterator state_iter = find(dfa.states.begin(), dfa.states.end(), new_setitem);
			if (state_iter == dfa.states.end())//没有在已有状态中找到该状态
			{
				dfa.states.push_back(new_setitem);
				dfa.goTo[Goto(i, next_symbol)] = dfa.states.size() - 1;
			}
			else//在已有状态中找到该状态
			{
				dfa.goTo[Goto(i, next_symbol)] = state_iter - dfa.states.begin();
			}
		}
	}
}

//获取action和goto
void Parser::get_action_goto()
{
	for (int i = 0; i < dfa.states.size(); i++)//遍历每一个状态
	{
		for (set<Item>::iterator iter = dfa.states[i].begin(); iter != dfa.states[i].end(); iter++)//对于每一个项目
		{
			if (productions[iter->pid].right.size() == iter->pointidx)//.在项目产生式的最右边，即是一个规约项目
			{
				set<string> thefollow = follow[productions[iter->pid].left];//获取左式的follow集
				for (set<string>::iterator follow_iter = thefollow.begin(); follow_iter != thefollow.end(); follow_iter++)
				{
					if (action_goto.count(Goto(i, *follow_iter)) == 1) output_error("移进归约冲突");
					else if (iter->pid == 0)
					{
						action_goto[Goto(i, *follow_iter)] = Move{accept, iter->pid };
					}
					else
					{
						action_goto[Goto(i, *follow_iter)] = Move{reduct, iter->pid };
					}
				}
			}
			else
			{
				string next_symbol = productions[iter->pid].right[iter->pointidx];
				Goto thegoto = { i, next_symbol };
				if (dfa.goTo.find(thegoto) != dfa.goTo.end())//如果有该状态转移
				{
					if (true == is_terminal(next_symbol))//如果转移符为终结符则移进
					{
						action_goto[thegoto] = Move{ shift, dfa.goTo[thegoto] };
					}
					else//如果转移符为非终结符则GOTO
					{
						// action_goto[thegoto] = Move{ _goto, dfa.goTo[thegoto] };
						action_goto[thegoto] = Move{ shift, dfa.goTo[thegoto] };
					}
				}
				else//如果没有该状态转移则出错
				{
					action_goto[thegoto] = Move{ error, dfa.goTo[thegoto] };
				}
			}
		}
	}
}

//输出DFA
void Parser::output_dfa(const char* filename)
{
	ofstream fout;
	fout.open(filename);
	if (!fout.is_open()) output_error("文件" + string(filename) + "打开失败");
	for (int i = 0; i < dfa.states.size(); i++)
	{
		fout << "I" << i << "= [";
		for (set<Item>::iterator iter = dfa.states[i].begin(); iter != dfa.states[i].end(); iter++)
		{
			fout << "{";
			Production p = productions[iter->pid];
			fout << p.left << " -> ";
			for (int i = 0; i < p.right.size();++i)
			{
				if (i == iter->pointidx)
				{
					fout << ". ";
				}
				fout << p.right[i] << " ";
			}
			if (p.right.size() == iter->pointidx)
			{
				fout << ". ";
			}
			fout << "}";
		}
		fout << "]" << endl << endl;
	}
	fout.close();
}

//输出action_goto
void Parser::output_action_goto(const char* filename)
{
	ofstream fout;
	fout.open(filename);
	if (!fout.is_open()) output_error("文件" + string(filename) + "打开失败");
	for (map<Goto, Move>::iterator iter = action_goto.begin(); iter != action_goto.end(); iter++)
	{
		if (iter->second.act != error)
		{
			fout << "status:" << iter->first.first << " read_stmbol:" << iter->first.second << " action:" << iter->second.act << " reach_status:" << iter->second.nextstate << endl;
		}
	}
}

//语法分析
void Parser::analyse(const char* filename)
{
	ifstream fin;
	fin.open(filename, ios::in);
	if (!fin.is_open()) output_error("文件" + string(filename) + "打开失败");
	symbol_stack.push("#");
	state_stack.push(0);
	char buf[1024];
	vector<string> words;
	vector<string> names;
	while (fin >> buf) 
	{
		if(buf == "ID" || buf == "NUM" )
		{
			words.push_back(buf);
			string buf_name;
			fin >> buf_name;
			names.push_back(buf_name);
		}
		else
		{
			words.push_back(buf);
			names.push_back("Others");
		}
	}
	for (int i = 0; i < words.size(); ++i)
	{
		if (action_goto.count(Goto(state_stack.top(), words[i])) == 0) output_error("语法错误，不匹配的符号");
		Move move = action_goto[Goto(state_stack.top(), words[i])];
		if (move.act == shift)//如果为移进
		{
			symbol_stack.push(words[i]);
			state_stack.push(move.nextstate);
			cout << "符号：" << words[i] << " shift" << " 栈顶：" << symbol_stack.top() << " " << state_stack.top() << endl;
		}
		else if (move.act == reduct)//如果为归约
		{
			Production reduct_p = productions[move.nextstate];
			int popSymNum = reduct_p.right.size();
			while (popSymNum--)
			{
				symbol_stack.pop();
				state_stack.pop();
			}
			symbol_stack.push(reduct_p.left);
			Move reduct_move = action_goto[Goto(state_stack.top(), reduct_p.left)];
			if (reduct_move.act == _goto)
			{
				state_stack.push(reduct_move.nextstate);
				cout << "符号：" << words[i] << " reduct" << " 符号栈顶：" << symbol_stack.top() << " 状态栈顶：" << state_stack.top() << endl;
				--i;
			}
			else output_error("语法错误：不能goto");
		}
		else if (move.act == accept)//如果为接受
		{
			cout << "success" << endl;
			return;
		}
	}
	output_error("语法错误，结尾错误");
}

//输出错误
void Parser::output_error(string s)
{
	cerr << s << endl;
	//exit(-1);
	getchar();
	getchar();
}

//获取新的临时变量
string Parser::get_newtemp() 
{
	return string("T") + to_string(temp_var++);
}

//pop & get top
E Parser::pop_symbol() 
{
	E ret = symbol_stack_e.top();
	symbol_stack_e.pop();
	state_stack.pop();
	return ret;
}

//push
void Parser::push_symbol(E temp) 
{
	symbol_stack_e.push(temp);
	if (action_goto.count(Goto(state_stack.top(), temp.name)) == 0) 
	{
		output_error(string("语法错误5"));
	}
	Move move = action_goto[Goto(state_stack.top(), temp.name)];
	state_stack.push(move.nextstate);
}

//输出符号栈、状态栈
void Parser::output_stack(ofstream& out) 
{
	out << "symbol_stack:";
	stack<E> symbol_temp = symbol_stack_e;
	stack<E> symbol_other;
	while (!symbol_temp.empty()) 
	{
		symbol_other.push(symbol_temp.top());
		symbol_temp.pop();
	}
	while (!symbol_other.empty()) 
	{
		out << symbol_other.top().name << " ";
		symbol_other.pop();
	}
	out << endl;
	out << "state_stack:";
	stack<int>state_temp = state_stack;
	stack<int>state_other;
	while (!state_temp.empty()) 
	{
		state_other.push(state_temp.top());
		state_temp.pop();
	}
	while (!state_other.empty()) 
	{
		out << state_other.top() << " ";
		state_other.pop();
	}
	out << endl;
}

//寻找函数
Func Parser::search_func(string ID) 
{
	for (int i = 0; i < funcTable.size();++i)
	{
		if(funcTable[i].name==ID)
		{
			return funcTable[i];
		}
	}
	Func temp;
	temp.name = "null";
	return temp;
}

//寻找变量
Var Parser::search_var(string ID) 
{
	for (int i = 0; i < varTable.size();++i)
	{
		if(varTable[i].name==ID)
		{
			return varTable[i];
		}
	}
	Var temp;
	temp.name = "null";
	return temp;
}

//合并
list<int> Parser::merge(list<int>& l1, list<int>& l2) 
{
	list<int> ret;
	ret.assign(l1.begin(), l1.end());
	ret.splice(ret.end(), l2);
	return ret;
}

//语义分析和生成中间代码
void Parser::analyse_e(const char* filename, ofstream& out) 
{
	bool acc = false;
	symbol_stack_e.push(E("#"));
	state_stack.push(0);
	ifstream fin;
	fin.open(filename, ios::in);
	if (!fin.is_open()) output_error("文件" + string(filename) + "打开失败");
	string buf;
	vector<string> words;
	vector<string> names;
	while (fin >> buf) 
	{
		if(buf == "ID" || buf == "NUM" )
		{
			words.push_back(buf);
			string buf_name;
			fin >> buf_name;
			names.push_back(buf_name);
		}
		else
		{
			words.push_back(buf);
			names.push_back("Others");
		}
	}

	for (int i = 0; i < words.size();)
	{
		output_stack(out);
		// cout << "state:" << state_stack.top() << " words:" << words[i]<< "\n";
		if (action_goto.count(Goto(state_stack.top(), words[i])) == 0) 
		{
			cout << words[i];
			output_error(string("语法错误1"));	
		}	
		Move move = action_goto[Goto(state_stack.top(), words[i])];
		if (move.act == shift) 
		{
			symbol_stack_e.push(E(words[i],names[i]));
			state_stack.push(move.nextstate);
			++i;
		}
		else if (move.act == reduct) 
		{
			Production reduct_p = productions[move.nextstate];
			switch (move.nextstate) 
			{
			case 3://declare ::= int ID M A function_declare
			{
				E function_declare = pop_symbol();
				E A = pop_symbol();
				E m = pop_symbol();
				E ID = pop_symbol();
				E _int = pop_symbol();
				funcTable.push_back(Func{ ID.real_name, D_INT, function_declare.plist, m.quad });
				push_symbol(E(reduct_p.left));
				break;
			}
			case 4://declare ::= int ID var_declare
			{
				E var_declare = pop_symbol();
				E ID = pop_symbol();
				E _int = pop_symbol();
				varTable.push_back(Var{ ID.real_name, D_INT });
				push_symbol(E(reduct_p.left));
				break;
			}
			case 5://declare ::= void ID M A function_declare
			{
				E function_declare = pop_symbol();
				E A = pop_symbol();
				E m = pop_symbol();
				E ID = pop_symbol();
				E _void = pop_symbol();
				funcTable.push_back(Func{ ID.real_name, D_VOID, function_declare.plist, m.quad });
				push_symbol(E(reduct_p.left));
				break;
			}
			case 6://A ::=
			{
				push_symbol(E(reduct_p.left));
				break;
			}
			case 8://function_declare ::= ( parameter ) sentence_block
			{
				E sentence_block = pop_symbol();
				E rparen = pop_symbol();
				E parameter = pop_symbol();
				E lparen = pop_symbol();
				E function_declare = E(reduct_p.left);
				function_declare.plist.assign(parameter.plist.begin(), parameter.plist.end());
				push_symbol(function_declare);
				break;
			}
			case 9://parameter :: = parameter_list
			{
				E parameter_list = pop_symbol();
				E parameter = E(reduct_p.left);
				parameter.plist.assign(parameter_list.plist.begin(), parameter_list.plist.end());
				push_symbol(parameter);
				break;
			}
			case 10://parameter ::= void
			{
				E _void = pop_symbol();
				E parameter = E(reduct_p.left);
				push_symbol(parameter);
				break;
			}
			case 11://parameter_list ::= param
			{
				E param = pop_symbol();
				E parameter_list = E(reduct_p.left);
				parameter_list.plist.push_back(D_INT);
				push_symbol(parameter_list);
				break;
			}
			case 12://parameter_list1 ::= param , parameter_list2
			{
				E parameter_list2 = pop_symbol();
				E comma = pop_symbol();
				E param = pop_symbol();
				E parameter_list1 = E(reduct_p.left);
				parameter_list2.plist.push_front(D_INT);
				parameter_list1.plist.assign(parameter_list2.plist.begin(), parameter_list2.plist.end());
				push_symbol(parameter_list1);
				break;
			}
			case 13://param ::= int ID
			{
				E ID = pop_symbol();
				E _int = pop_symbol();
				varTable.push_back(Var{ ID.real_name,D_INT });
				code.emit("get", "_", "_", ID.real_name);
				push_symbol(E(reduct_p.left));
				break;
			}
			case 14://sentence_block ::= { inner_declare sentence_list }
			{
				E rbrace = pop_symbol();
				E sentence_list = pop_symbol();
				E inner_declare = pop_symbol();
				E lbrace = pop_symbol();
				E sentence_block = E(reduct_p.left);
				sentence_block.nextList = sentence_list.nextList;
				int popNum = 0;
				for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++) 
				{
					if (riter->level > now_level)
						popNum++;
					else
						break;
				}
				for (int i = 0; i < popNum; i++) 
				{
					varTable.pop_back();
				}
				push_symbol(sentence_block);
				break;
			}
			case 17://inner_var_declare ::= int ID
			{
				E ID = pop_symbol();
				E _int = pop_symbol();
				push_symbol(E(reduct_p.left));
				varTable.push_back(Var{ ID.real_name,D_INT});
				break;
			}
			case 18://sentence_list ::= sentence M sentence_list
			{
				E sentence_list2 = pop_symbol();
				E m = pop_symbol();
				E sentence = pop_symbol();
				E sentence_list1 = E(reduct_p.left);
				sentence_list1.nextList = sentence_list2.nextList;
				code.back_patch(sentence.nextList, m.quad);
				push_symbol(sentence_list1);
				break;
			}
			case 19://sentence_list ::= sentence
			{
				E sentence = pop_symbol();
				E sentence_list = E(reduct_p.left);
				sentence_list.nextList = sentence.nextList;
				push_symbol(sentence_list);
				break;
			}
			case 20://sentence ::= if_sentence
			{
				E if_sentence = pop_symbol();
				E sentence = E(reduct_p.left);
				sentence.nextList = if_sentence.nextList;
				push_symbol(sentence);
				break;
			}
			case 21://sentence ::= while_sentence
			{
				E while_sentence = pop_symbol();
				E sentence = E(reduct_p.left);
				sentence.nextList = while_sentence.nextList;
				push_symbol(sentence);
				break;
			}
			case 22://sentence ::= return_sentence
			{
				E return_sentence = pop_symbol();
				E sentence = E(reduct_p.left);
				push_symbol(sentence);
				break;
			}
			case 23://sentence ::= assign_sentence
			{
				E assign_sentence = pop_symbol();
				E sentence = E(reduct_p.left);
				push_symbol(sentence);
				break;
			}
			case 24://assign_sentence ::= ID = expression ;
			{
				E comma = pop_symbol();
				E expression = pop_symbol();
				E assign = pop_symbol();
				E ID = pop_symbol();
				E assign_sentence = E(reduct_p.left);
				code.emit("=", expression.real_name, "_", ID.real_name);
				push_symbol(assign_sentence);
				break;
			}
			case 25://return_sentence ::= return ;
			{
				E comma = pop_symbol();
				E _return = pop_symbol();
				code.emit("return", "_", "_", "_");
				push_symbol(E(reduct_p.left));
				break;
			}
			case 26://return_sentence ::= return expression ;
			{
				E comma = pop_symbol();
				E expression = pop_symbol();
				E _return = pop_symbol();
				code.emit("return", expression.real_name, "_", "_");
				push_symbol(E(reduct_p.left));
				break;
			}
			case 27://while_sentence ::= while M ( expression ) A sentence_block
			{
				E sentence_block = pop_symbol();
				E A = pop_symbol();
				E rparen = pop_symbol();
				E expression = pop_symbol();
				E lparen = pop_symbol();
				E m = pop_symbol();
				E _while = pop_symbol();
				E while_sentence = E(reduct_p.left);
				code.back_patch(sentence_block.nextList, m.quad);
				while_sentence.nextList = expression.falseList;
				code.emit("j", "_", "_", to_string(m.quad));
				push_symbol(while_sentence);
				break;
			}
			case 28://if_sentence ::= if ( expression ) A sentence_block
			{
				E sentence_block = pop_symbol();
				E A = pop_symbol();
				E rparen = pop_symbol();
				E expression = pop_symbol();
				E lparen = pop_symbol();
				E _if = pop_symbol();
				E if_sentence = E(reduct_p.left);
				expression.falseList.splice(expression.falseList.begin(), sentence_block.nextList);
				if_sentence.nextList = expression.falseList;
				push_symbol(if_sentence);
				break;
			}
			case 29://if_sentence ::= if ( expression ) A1 sentence_block1 N else M A2 sentence_block2
			{
				E sentence_block2 = pop_symbol();
				E A2 = pop_symbol();
				E m = pop_symbol();
				E _else = pop_symbol();
				E n = pop_symbol();
				E sentence_block1 = pop_symbol();
				E A1 = pop_symbol();
				E rparen = pop_symbol();
				E expression = pop_symbol();
				E lparen = pop_symbol();
				E _if = pop_symbol();
				E if_sentence = E(reduct_p.left);
				code.back_patch(expression.falseList, m.quad);
				if_sentence.nextList = merge(sentence_block1.nextList, sentence_block2.nextList);
				if_sentence.nextList = merge(if_sentence.nextList, n.nextList);
				push_symbol(if_sentence);
				break;
			}
			case 30://N ::= 
			{
				E n = E(reduct_p.left);
				n.nextList.push_back(code.get_nextquad());
				code.emit("j", "_", "_", "-1");
				push_symbol(n);
				break;
			}
			case 31://M ::=
			{
				E m = E(reduct_p.left);
				m.quad = code.get_nextquad();
				push_symbol(m);
				break;
			}
			case 32://expression ::= add_expression
			{
				E add_expression = pop_symbol();
				E expression = E(reduct_p.left);
				expression.real_name = add_expression.real_name;
				push_symbol(expression);
				break;
			}
			case 33://expression ::= add_expression1 > add_expression2
			{
				E add_expression2 = pop_symbol();
				E gt = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j<=", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 34://expression ::= add_expression1 < add_expression2
			{
				E add_expression2 = pop_symbol();
				E lt = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j>=", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 35://expression ::= add_expression1 == add_expression2
			{
				E add_expression2 = pop_symbol();
				E eq = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j!=", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 36://expression ::= add_expression1 >= add_expression2
			{
				E add_expression2 = pop_symbol();
				E get = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j<", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 37://expression ::= add_expression1 <= add_expression2
			{
				E add_expression2 = pop_symbol();
				E let = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j>", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 38://expression ::= add_expression1 != add_expression2
			{
				E add_expression2 = pop_symbol();
				E neq = pop_symbol();
				E add_expression1 = pop_symbol();
				E expression = E(reduct_p.left);
				expression.falseList.push_back(code.get_nextquad());
				code.emit("j==", add_expression1.real_name, add_expression2.real_name, "-1");
				push_symbol(expression);
				break;
			}
			case 39://add_expression ::= item
			{
				E item = pop_symbol();
				E add_expression = E(reduct_p.left);
				add_expression.real_name = item.real_name;
				push_symbol(add_expression);
				break;
			}
			case 40://add_expression1 ::= item + add_expression2
			{
				E add_expression2 = pop_symbol();
				E add = pop_symbol();
				E item = pop_symbol();
				E add_expression1 = E(reduct_p.left);
				add_expression1.real_name = get_newtemp();
				code.emit("+", item.real_name, add_expression2.real_name, add_expression1.real_name);
				push_symbol(add_expression1);
				break;
			}
			case 41://add_expression ::= item - add_expression
			{
				E add_expression2 = pop_symbol();
				E sub = pop_symbol();
				E item = pop_symbol();
				E add_expression1 = E(reduct_p.left);
				add_expression1.real_name = get_newtemp();
				code.emit("-", item.real_name, add_expression2.real_name, add_expression1.real_name);
				push_symbol(add_expression1);
				break;
			}
			case 42://item ::= factor
			{
				E factor = pop_symbol();
				E item = E(reduct_p.left);
				item.real_name = factor.real_name;
				push_symbol(item);
				break;
			}
			case 43://item1 ::= factor * item2
			{
				E item2 = pop_symbol();
				E mul = pop_symbol();
				E factor = pop_symbol();
				E item1 = E(reduct_p.left);
				item1.real_name = get_newtemp();
				code.emit("*", factor.real_name, item2.real_name, item1.real_name);
				push_symbol(item1);
				break;
			}
			case 44://item1 ::= factor / item2
			{
				E item2 = pop_symbol();
				E div = pop_symbol();
				E factor = pop_symbol();
				E item1 = E(reduct_p.left);
				item1.real_name = get_newtemp();
				code.emit("/", factor.real_name, item2.real_name, item1.real_name);
				push_symbol(item1);
				break;
			}
			case 45://factor ::= NUM
			{
				E num = pop_symbol();
				E factor = E(reduct_p.left);
				factor.real_name = num.real_name;
				push_symbol(factor);
				break;
			}
			case 46://factor ::= ( expression )
			{
				E rparen = pop_symbol();
				E expression = pop_symbol();
				E lparen = pop_symbol();
				E factor = E(reduct_p.left);
				factor.real_name = expression.real_name;
				push_symbol(factor);
				break;
			}
			case 47://factor ::= ID ( argument_list )
			{
				E rparen = pop_symbol();
				E argument_list = pop_symbol();
				E lparen = pop_symbol();
				E ID = pop_symbol();
				E factor = E(reduct_p.left);
				Func f = search_func(ID.real_name);
				if (f.name == "null")
				{
					output_error(string("语法错误2"));
				}
				else if (argument_list.alist.size()!=f.paramtype.size()) 
				{
					output_error(string("参数个数不匹配"));
				}
				else 
				{
					for (list<string>::iterator iter = argument_list.alist.begin(); iter != argument_list.alist.end(); iter++) 
					{
						code.emit("par", (*iter), "_", "_");
					}
					factor.real_name = get_newtemp();
					code.emit("call", ID.real_name, "_", "_");
					code.emit("=", "@RETURN_PLACE", "_", factor.real_name);

					push_symbol(factor);
				}
				break;
			}
			case 48://factor ::= ID
			{
				E ID = pop_symbol();
				if (search_var(ID.real_name).name == "null") 
				{
					output_error(string("语法错误4"));
				}
				E factor = E(reduct_p.left);
				factor.real_name = ID.real_name;
				push_symbol(factor);
				break;
			}
			case 49://argument_list ::= 
			{
				E argument_list = E(reduct_p.left);
				push_symbol(argument_list);
				break;
			}
			case 50://argument_list ::= expression
			{
				E expression = pop_symbol();
				E argument_list = E(reduct_p.left);
				argument_list.alist.push_back(expression.real_name);
				push_symbol(argument_list);
				break;
			}
			case 51://argument_list1 ::= expression , argument_list2
			{
				E argument_list2 = pop_symbol();
				E comma = pop_symbol();
				E expression = pop_symbol();
				E argument_list1 = E(reduct_p.left);
				argument_list2.alist.push_front(expression.real_name);
				argument_list1.alist.assign(argument_list2.alist.begin(), argument_list2.alist.end());
				push_symbol(argument_list1);
				break;
			}
			default:
				int pop_symbol_num = reduct_p.right.size();
				for (int i = 0; i < pop_symbol_num; i++) 
				{
					pop_symbol();
				}
				push_symbol(E(reduct_p.left));
				break;
			}
		}
		else if (move.act == accept)//P ::= N declare_list 
		{
			cout<<"成功";
			acc = true;
			Func f = search_func("program");
			pop_symbol();
			E n = pop_symbol();

			code.back_patch(n.nextList, f.startaddr);
			code.output_code("zcode.txt");
			break;
		}
	}
	if (!acc) 
	{
		output_error("语法错误：未知的结尾");
	}
}


void Parser::output_production(const char* filename)
{
	ofstream fout;
	fout.open(filename);
	if (!fout.is_open()) output_error("文件" + string(filename) + "打开失败");
	for (int i = 0; i < productions.size(); i++)
	{
		Production p = productions[i];
		fout << p.left << " -> ";
		for (int i = 0; i < p.right.size();++i)
		{
			fout << p.right[i] << " ";
		}
		fout << endl;
	}
	fout.close();
}

//主函数
int main()
{
	ofstream out("zresult.txt");
	Parser parser("zproductions.txt");
	//parser.output_action_goto("zaction_goto.txt");
	//parser.output_dfa("zdfa.txt");
	parser.analyse_e("zword.txt", out);
	getchar();
	return 0;
}

