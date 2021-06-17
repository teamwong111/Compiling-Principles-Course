#include "lextual.h"

//判断关键词
bool Lextual::isKeyWord(char a[])
{
	int len = strlen(a);
	for (int i = 0; i < num_keyword; ++i)
	{
		if (int(strlen(Keyword[i])) == len)
		{
			if (strcmp(a, Keyword[i]) == 0)
				return true;
		}
	}
	return false;
}
//判断单目运算符
bool Lextual::isUnaryOperator(char a)
{
	for (int i = 0; i < num_uo; ++i)
	{
		if (Unary_Operator[i] == a) return true;
	}
	return false;
}
//判断双目运算符
bool Lextual::isBinaryOperator(char a[])
{
	for (int i = 0; i < num_bo; ++i)
	{
		if (strcmp(Binary_Operator[i], a) == 0)
			return true;
	}
	return false;
}
//判断界符
bool Lextual::isBoundary(char a)
{
	for (int i = 0; i < num_boundary; ++i)
	{
		if (Boundary[i] == a) 
			return true;
	}
	return false;
}
//判断注释
bool Lextual::isAnotation(char a[])
{
	for (int i = 0; i < num_anotaion; ++i)
	{
		if (strcmp(a, Anotation[i]) == 0) 
			return true;
	}
	return false;
}
//0-9
bool Lextual::isNumber(char a)
{
	if (a >= '0' && a <= '9') 
		return true;
	return false;
}
//a-z, A-Z
bool Lextual::isLetter(char a)
{
	if ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z')) 
		return true;
	return false;
}
//生成dfa
Response Lextual::dfa(const char* resultname)
{
	string result = "result:\n";
	int res_state = 1;
	Response response;
	/*
		result - 记录本次词法分析的结果，可以正确也可以错误
		lextualResult - 记录正确的结果输出到文件里，为了下一步语法分析
	*/
	string lextualResult = "";

	char str[100];
	string Qstr = "";
	char ch;
	int point;

	ch = fgetc(file);
	bool finish = false;
	while (!finish)
	{
		point = 0;
		if (isNumber(ch))
		{
			state = s_num;
			Qstr += ch;
			str[point++] = ch;
			ch = fgetc(file);
			while (isLetter(ch) || isNumber(ch) || ch == '.')
			{
				Qstr += ch;
				str[point++] = ch;
				ch = fgetc(file);
			}
			str[point] = '\0';
			for (int i = 0; i < point; ++i)
			{
				switch (state)
				{
				case s_num:
					//dfa随着功能扩展需要扩展，比如识别复数、科学技术etc
					if (isNumber(str[i])) {
						state = s_num;
					}
					else {
						state = s_reject;
					}
					break;

				case s_letter:
					state = s_reject;
					break;

				case s_reject:
					state = s_reject;
					break;
				}
			}
			if (state == s_num)
			{
				result += Qstr + " 常量\n";
				lextualResult += "NUM " + Qstr + " ";
			}
			else
			{
				result += Qstr + " 出错，不是常量\n";
				res_state = 0;
			}
			point = 0;
			Qstr = "";
		}
		if (isLetter(ch))
		{
			state = s_letter;
			str[point++] = ch;
			Qstr += ch;
			ch = fgetc(file);
			while (isLetter(ch) || isNumber(ch))
			{
				str[point++] = ch;
				Qstr += ch;
				ch = fgetc(file);
			}
			str[point] = '\0';

			if (isKeyWord(str))
			{
				result += Qstr + " 关键字\n";
				lextualResult += Qstr + " ";
			}
			else
			{
				//DFA to recognize 标识符
				for (int i = 0; i < point; ++i)
				{
					switch (state)
					{
					case s_num:
						if (isNumber(str[i])) {
							state = s_num;
						}
						else {
							state = s_reject;
						}
						break;

					case s_letter:
						if (isLetter(str[i])) {
							state = s_letter;
						}
						else if (isNumber(str[i])) {
							state = s_num;
						}
						else {
							state = s_reject;
						}
						break;

					case s_reject:
						state = s_reject;
						break;
					}
				}

				if (state != s_reject)
				{
					result += Qstr + " 标识符\n";
					lextualResult += "ID " + Qstr + " ";

				}
				else
				{
					result += Qstr + " 出错，不是标识符\n";
					res_state = 0;
				}
			}

			point = 0;
			Qstr = "";
		}
		if (isBoundary(ch))
		{
			result += string(1, ch) + " 界符\n";
			lextualResult += ch;
			lextualResult += " ";

			if ((ch = fgetc(file)) == EOF)
			{
				finish = true;
				break;
			}
		}
		if (isUnaryOperator(ch))
		{
			if (ch == '/')
			{
				state = s_ano1;
				ch = fgetc(file);

				bool finish = false;
				while (!finish)
				{
					switch (state)
					{
					case s_ano1:
						if (ch == '/') {
							state = s_ano2;
							ch = fgetc(file);
						}
						else if (ch == '*') {
							state = s_ano5;
							ch = fgetc(file);
						}
						else {
							state = s_ano4;
							ch = fgetc(file);
						}
						break;

					case s_ano2:
						if (ch == '\n') {
							state = s_ano3;
							ch = fgetc(file);
						}
						else {
							state = s_ano2;
							ch = fgetc(file);
						}
						break;

					case s_ano3:
						state = s_ano3;
						finish = true;//接受态，退出循环
						break;

					case s_ano4:
						state = s_ano4;
						finish = true;//到达拒绝态，退出循环
						result += "/ 单目运算符\n";
						lextualResult += "/ ";
						break;

					case s_ano5:
						if (ch == '*') {
							state = s_ano6;
							ch = fgetc(file);
						}
						else {
							state = s_ano5;
							//在这里要判断是否结束
							if ((ch = fgetc(file)) == EOF)
							{
								finish = true;
							}
						}
						break;

					case s_ano6:
						if (ch == '/') {
							state = s_ano3;
							ch = fgetc(file);
						}
						else {
							state = s_ano5;
							ch = fgetc(file);
						}
						break;
					}
				}
			}
			else
			{
				str[point++] = ch;
				Qstr += ch;
				if ((ch = fgetc(file)) == EOF)
				{
					finish = true;
				}
				str[point++] = ch;
				Qstr += ch;
				str[point] = '\0';
				// Qstr += '\0';
				if (finish == false && isBinaryOperator(str))
				{
					result += Qstr + " 双目运算符\n";
					lextualResult += Qstr;
					lextualResult += " ";
					ch = fgetc(file);
				}
				else
				{
					result += Qstr[0] + " 单目运算符\n";
					lextualResult += str[0];
					lextualResult += " ";
				}
			}

			point = 0;
			Qstr = "";
		}
		if (ch == ' ' || ch == '\n' || ch == '\t')
		{
			if ((ch = fgetc(file)) == EOF)
			{
				finish = true;
				break;
			}
			continue;
		}
		if (ch == EOF)
		{
			finish = true;
		}
	}

	ofstream out;
	out.open(resultname, ios::out);
	if (!out.is_open())
	{
		response.state = 0;
		return response;
	}

	if (res_state == 1)
	{
		out << lextualResult;
		out.close();
		cout << "词法分析成功" << endl;
	}
	else
	{
		out << "failure!";
		out.close();
		exit(-1);
	}
	response.result = result;
	response.state = res_state;
	return response;
}
//读取文件
void Lextual::readfile(const char* filename)
{
	file = fopen(filename, "r+");
}
//获取词法分析结果
Response Lextual::getResult(const char* filename, const char* resultname)
{
	Response response;
	readfile(filename);
	response = dfa(resultname);
	return response;
}
